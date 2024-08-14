//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

#include <GenericCompositeModel.h>
#include <Recipe.h>
#include <RotatedFrame.h>
#include <TranslatedFrame.h>
#include <Singleton.h>
#include <OMModel.h>
#include <CompositeElement.h>
#include <cassert>
#include <Logger.h>

#ifdef PYTHON_SCRIPT_SUPPORT
#  include <ScriptLoader.h>
#endif // PYTHON_SCRIPT_SUPPORT

using namespace RZ;

std::list<std::string>
GenericEvaluator::symbols() const
{
  std::list<std::string> list;

  for (auto p : *m_dict)
    list.push_back(p.first);

  return list;
}

bool
GenericEvaluator::registerCustomFunction(GenericCustomFunction *func)
{
  m_funcList.push_back(func);

  return true;
}

std::list<GenericCustomFunction *>
GenericEvaluator::functions() const
{
  return m_funcList;
}

ExprRandomState *
GenericEvaluator::randState() const
{
  return m_randState;
}

Real *
GenericEvaluator::resolve(std::string const &symbol)
{
  auto it = m_dict->find(symbol);

  if (it != m_dict->end())
    return &(*m_dict)[symbol]->value;

  return nullptr;
}

GenericEvaluator::GenericEvaluator(
  GenericEvaluatorSymbolDict *dict,
  ExprRandomState *state)
{
  m_dict = dict;

  if (state == nullptr) {
    m_ownState = new ExprRandomState();
    state = m_ownState;
  }

  m_randState = state;
}

GenericEvaluator::~GenericEvaluator()
{
  if (m_ownState != nullptr)
    delete m_ownState;
}

GenericComponentParamEvaluator::~GenericComponentParamEvaluator()
{
  if (evaluator != nullptr)
    delete evaluator;
}

void
GenericComponentParamEvaluator::assign()
{
  std::string param;

  if (description == nullptr)
    throw std::runtime_error("Param evaluator has no description");

  if (description->s_target == -1)
    throw std::runtime_error("Param evaluator has undefined object index");

  // Positional argument. Lazy resolution of its name.
  if (position != -1) {
    unsigned actualIndex = position + element->hidden();
    auto props = element->sortedProperties();

    if (actualIndex >= props.size())
      throw std::runtime_error("Too many properties passed to element");
    
    description->parameter = props[actualIndex];
    position = -1;
  }

  param = description->parameter;

  // An evaluator exists, this string must be evaluated as such
  if (evaluator != nullptr) {
    Real value;

    // Hey, butler, stop butling yourself!
    value = evaluator->evaluate();

    switch (type) {
      case GENERIC_MODEL_PARAM_TYPE_ELEMENT:
        element->set(param, value);
        break;

      case GENERIC_MODEL_PARAM_TYPE_ROTATED_FRAME:
        if (param == "angle")
          rotation->setAngle(deg2rad(value));
        else if (param == "eX")
          rotation->setAxisX(value);
        else if (param == "eY")
          rotation->setAxisY(value);
        else if (param == "eZ")
          rotation->setAxisZ(value);
        else
          throw std::runtime_error("Unknown rotation parameter `" + param + "'");
        
        rotation->recalculate();
        break;

      case GENERIC_MODEL_PARAM_TYPE_TRANSLATED_FRAME:
        if (param == "dX")
          translation->setDistanceX(value);
        else if (param == "dY")
          translation->setDistanceY(value);
        else if (param == "dZ")
          translation->setDistanceZ(value);
        else
          throw std::runtime_error("Unknown translation parameter `" + param + "'");

        translation->recalculate();
        break;
    }

    // This has some storage (it is exposed as a variable somehow), update
    // contents accordingly
    if (storage != nullptr) {
      if (!storage->test(value)) {
        RZWarning(
          "Cannot assign `%s': evaluated expression is out of bounds\n",
          param.c_str());
      } else {
        storage->value = value;
        // Storage changed, assign recursively

        for (auto p : storage->dependencies)
          p->assign();
      }
    }
  } else {
    switch (type) {
      case GENERIC_MODEL_PARAM_TYPE_ELEMENT:
        element->set(param, assignString);
        break;

      default:
        throw std::runtime_error("Reference frames do not accept string parameters");
    }
  }
}

bool
GenericModelParam::test(Real val)
{
  if (description == nullptr)
    return true;
  
  return (description->min <= val) && (val <= description->max);
}

GenericCompositeModel::GenericCompositeModel(
  Recipe *recipe,
  OMModel *model,
  GenericCompositeModel *parentModel,
  Element *parent)
{
  m_recipe      = recipe;
  m_model       = model;
  m_parentModel = parentModel;
  m_parent      = parent;
  m_randState   = &m_ownState;

  assert(m_model != nullptr);
}

GenericCompositeModel::~GenericCompositeModel()
{
  for (auto p : m_expressions)
    delete p;

  for (auto p : m_genParamStorage)
    delete p;

  for (auto p : m_customFactoryList)
    delete p;
}

std::list<std::string>
GenericCompositeModel::params() const
{
  std::list<std::string> params;

  for (auto p : m_recipe->params())
    params.push_back(p.first);

  return params;

}

std::list<std::string>
GenericCompositeModel::dofs() const
{
  std::list<std::string> dofs;

  for (auto p : m_recipe->dofs())
    dofs.push_back(p.first);

  return dofs;
}

GenericModelParam *
GenericCompositeModel::lookupParam(std::string const &param)
{
  auto it = m_params.find(param);

  if (it == m_params.end())
    return nullptr;

  return m_params[param];
}

GenericModelParam *
GenericCompositeModel::lookupDof(std::string const &dof)
{
  auto it = m_dofs.find(dof);

  if (it == m_dofs.end())
    return nullptr;

  return m_dofs[dof];
}

GenericCompositeModel *
GenericCompositeModel::parentCompositeModel() const
{
  return m_parentModel;
}

void
GenericCompositeModel::updateRandState()
{
  for (auto m : m_elements)
    if (m->nestedCompositeModel() != nullptr)
      m->nestedCompositeModel()->updateRandState();
      
  m_randState->update();
  assignEverything();
}

void
GenericCompositeModel::setRandomState(ExprRandomState *state)
{
  m_randState = state;
}

std::string
GenericCompositeModel::resolveFilePath(std::string const &path) const
{
  std::string absPath;

  // Relative filename
  if (path[0] != '/') {
    if (m_recipe != nullptr) {
      for (auto &p : m_recipe->searchPaths()) {
        absPath = p + "/" + path;
        if (access(absPath.c_str(), F_OK) != -1)
          return absPath;
      }
    }

    return "";
  }

  return path;
}

void
GenericCompositeModel::assignEverything()
{
  for (auto p : m_expressions)
    p->assign();
}

bool
GenericCompositeModel::loadScript(std::string const &path)
{
#ifdef PYTHON_SCRIPT_SUPPORT
  ScriptLoader *loader = ScriptLoader::instance();

  if (loader == nullptr) {
    RZError("Failed to acquire ScriptLoader singleton\n");
    return false;
  }

  Script *script = loader->load(path);

  if (script == nullptr)
    return false;

  m_scripts.push_back(script);
  return true;

#else
  RZError(
    "Cannot load script `%s': Python support disabled at compile time\n",
    path.c_str());
  return false;
#endif
}

bool
GenericCompositeModel::setParam(std::string const &name, Real value)
{
  GenericModelParam *param = lookupParam(name);

  if (param == nullptr)
    throw std::runtime_error("Unknown parameter `" + name + "'");

  if (std::isnan(value)) {
    RZError(
      "Parameter `%s': attempting to set with a value that is not a number\n",
      name.c_str());
    return false;
  }

  if (!param->test(value)) {
    RZWarning(
      "Parameter `%s': value %g out of range (%g, %g)\n",
      name.c_str(),
      value,
      param->description->min,
      param->description->max);
    return false;
  }

  param->value = value;

  for (auto p : param->dependencies)
    p->assign();

  return true;
}

bool
GenericCompositeModel::setDof(std::string const &name, Real value)
{
  GenericModelParam *dof = lookupDof(name);

  if (dof == nullptr)
    throw std::runtime_error("Unknown parameter `" + name + "'");

  if (std::isnan(value)) {
    RZError(
      "DOF `%s': attempting to set with a value that is not a number\n",
      name.c_str());
    return false;
  }

  if (!dof->test(value)) {
    RZWarning(
      "DOF `%s': value %g out of range (%g, %g)\n",
      name.c_str(),
      value,
      dof->description->min,
      dof->description->max);
    return false;
  }

  dof->value = value;

  for (auto p : dof->dependencies)
    p->assign();

  return true;
}

// Takes the recipe and constructs elements
void
GenericCompositeModel::delayedCreationLoop()
{
  unsigned int elCount  = m_recipe->elements().size();
  unsigned int ctxCount = m_recipe->contexts().size();

  while (m_completedElements < elCount || m_completedFrames < ctxCount) {
    unsigned prevElements = m_completedElements;
    unsigned prevFrames   = m_completedFrames;

    resolvePorts();           // Resolve element ports
    createFrames(nullptr);    // Create frames that may be defined on these ports
    createDelayedElements();  // Create elements depending on the previous frames
    
    if (m_completedElements < elCount && m_completedElements == prevElements)
      throw std::runtime_error("Some elements are self-contained");
    
    if (m_completedFrames < ctxCount && m_completedFrames == prevFrames)
      throw std::runtime_error("Some reference frames are self-contained");
  }
}

void
GenericCompositeModel::build(
  ReferenceFrame *parent,
  std::string const &prefix)
{
  // 1. Loop to create frames
  // 2. Loop to create elements
  // 3. Loop to register dofs and params
  // 4. Loop to compile expressions
  //    4.1 For each reference frame (in tree), create symbol dict
  //    4.2 Call makeEvaluator
  // 5. Loop over plug constructs
  // 6. Define optical paths

  m_prefix = prefix;
  
  createParams();
  loadScripts();
  initGlobalScope();
  registerCustomElements();
  createFrames(parent);
  createElements(parent);
  delayedCreationLoop();
  createExpressions();
  exposeOpticalPaths();
  exposePorts();
  assignEverything();
}

void
GenericCompositeModel::resolvePorts()
{
  auto contexts = m_recipe->contexts();
  auto firstCtx = contexts[0];
  
  for (size_t i = 1; i < contexts.size(); ++i) {
    if (contexts[i]->type == RECIPE_CONTEXT_TYPE_PORT && m_frames[i] == nullptr) {
      Element *element = m_elements[contexts[i]->element->s_index];
      if (element != nullptr) {
        ReferenceFrame *frame = element->getPortFrame(contexts[i]->port);
        if (frame == nullptr)
          throw std::runtime_error(
            "Element `" 
            + element->name() 
            + "' has no port `" 
            + contexts[i]->port 
            + "'");
        ++m_completedFrames;
        m_frames[i] = frame;
      }
    }
  }
}

ReferenceFrame *
GenericCompositeModel::getFrameOfContext(const RecipeContext *ctx) const
{
  ReferenceFrame *pFrame;

  switch (ctx->type) {
    case RECIPE_CONTEXT_TYPE_ROOT:
      return m_frames[0];

    case RECIPE_CONTEXT_TYPE_PORT:
    case RECIPE_CONTEXT_TYPE_ROTATION:
    case RECIPE_CONTEXT_TYPE_TRANSLATION:
      return m_frames[ctx->s_index];
  }
  
  return nullptr;
}

bool
GenericCompositeModel::registerCustomFactory(CompositeElementFactory *factory)
{
  // Check if factory already exists
  if (m_customFactories.find(factory->name()) != m_customFactories.cend())
    return false;

  m_customFactoryList.push_back(factory);
  m_customFactories[factory->name()] = factory;

  return true;
}

ElementFactory *
GenericCompositeModel::lookupElementFactory(const std::string &name, bool &custom) const
{
  auto p = m_customFactories.find(name);
  custom = p != m_customFactories.end();

  if (custom) {
    return p->second;
  } else {
    if (m_parentModel == nullptr) {
      Singleton *sing = Singleton::instance();
      return sing->lookupElementFactory(name);
    } else {
      return m_parentModel->lookupElementFactory(name, custom);
    }
  }
}

void
GenericCompositeModel::registerCustomElements()
{
  Singleton *sing = Singleton::instance();
  auto elements = m_recipe->customElements();

  for (auto p : elements) {
    auto factory = new CompositeElementFactory(p.first, p.second, this);
    if (!registerCustomFactory(factory)) {
      delete factory;
      throw std::runtime_error("Attempting to register custom factory `" + p.first + "' twice. THIS IS A BUG.");
    }
  }
}

void
GenericCompositeModel::createFrames(ReferenceFrame *parent)
{
  auto contexts = m_recipe->contexts();
  auto firstCtx = contexts[0];
  std::string name;

  if (m_frames.size() == 0) {
    auto size = contexts.size();
    m_frames.resize(size);

    for (auto i = 0; i < size; ++i)
      m_frames[i] = nullptr;
    
    m_completedFrames = 1;
    m_frames[0] = parent;
  }
  

  for (size_t i = 1; i < contexts.size(); ++i) {
    ReferenceFrame *pFrame = getFrameOfContext(contexts[i]->parent);

    // If we do not have this parent or it has been already created, skip
    if (pFrame == nullptr || m_frames[i] != nullptr)
      continue;
    
    name = m_prefix + contexts[i]->name;

    switch (contexts[i]->type) {
      case RECIPE_CONTEXT_TYPE_ROOT:
        throw std::runtime_error("Root context in wrong place");
        break;

      case RECIPE_CONTEXT_TYPE_PORT:
        continue;
      
      case RECIPE_CONTEXT_TYPE_ROTATION:
        m_frames[i] = new RotatedFrame(name, pFrame, Vec3::eZ(), 0);
        break;

      case RECIPE_CONTEXT_TYPE_TRANSLATION:
        m_frames[i] = new TranslatedFrame(name, pFrame, Vec3::zero());
        break;
    }

    ++m_completedFrames;
    if (!m_model->registerFrame(m_frames[i]))
      throw std::runtime_error("Reference frame `" + name + "' already exists");
  }
}

void
GenericCompositeModel::createElementInside(
  RecipeElementStep *step,
  ReferenceFrame *pFrame)
{
  std::string baseName = step->name;
  if (baseName.size() == 0)
    baseName = step->factory;
  std::string name = m_prefix + baseName;
  int index = step->s_index;
  bool custom;
  auto factory = lookupElementFactory(step->factory, custom);

  if (factory == nullptr)
    throw std::runtime_error("Undefined element class `" + step->factory + "'");

  m_elements[index] = factory->make(name, pFrame, m_parent);
  m_elements[index]->setParentModel(this);
  
  ++m_completedElements;

  if (!m_model->autoRegisterElement(m_elements[index]))
    throw std::runtime_error("Element`" + name + "' already exists");

  if (step->factory == "Detector") {
    // Detectors must be notified accordingly
    Detector *det = static_cast<Detector *>(m_elements[index]);
    notifyDetector(det->name(), det);
  }
}

void
GenericCompositeModel::createElements(ReferenceFrame *parent)
{
  auto elements = m_recipe->elements();
  auto contexts = m_recipe->contexts();
  auto firstCtx = contexts[0];
  
  std::string name;

  if (m_elements.size() == 0) {
    m_elements.resize(elements.size());
    for (auto i = 0; i < elements.size(); ++i)
      m_elements[i] = nullptr;
  }
  
  for (size_t i = 0; i < elements.size(); ++i) {
    if (elements[i]->delayedCreation) {
      // This one is created during a plug step
      m_elements[i] = nullptr;
      continue;
    }

    ReferenceFrame *pFrame = getFrameOfContext(elements[i]->parent);
    
    createElementInside(elements[i], pFrame);
  }
}

void
GenericCompositeModel::createDelayedElements()
{
  auto elements = m_recipe->elements();
  auto contexts = m_recipe->contexts();
  auto firstCtx = contexts[0];
  
  std::string name;

  if (m_elements.size() == 0) {
    m_elements.resize(elements.size());
    for (auto i = 0; i < elements.size(); ++i)
      m_elements[i] = nullptr;
  }

  for (size_t i = 1; i < elements.size(); ++i) {
    Element *element = m_elements[elements[i]->s_index];
    if (elements[i]->delayedCreation && element == nullptr) {
      ReferenceFrame *pFrame = getFrameOfContext(elements[i]->parent);

      if (pFrame != nullptr)
        createElementInside(elements[i], pFrame);
    }
  }
}

GenericModelParam *
GenericCompositeModel::allocateParam()
{
  GenericModelParam *param = new GenericModelParam();

  m_genParamStorage.push_back(param);

  return param;
}

void
GenericCompositeModel::createParams()
{
  auto &dofs   = m_recipe->dofs();
  auto &params = m_recipe->params();

  // This creates the parameters for DOFs and model parameters. This is
  // something that should be available in the global scope.

  // The usage of references is mandatory here.
  for (auto &dof : dofs) {
    auto genP = allocateParam();
    genP->description = &dof.second;
    genP->value       = genP->description->defaultVal;
    m_dofs[dof.first] = genP;
    registerDof(dof.first, genP);
  }

  for (auto &param : params) {
    auto genP = allocateParam();
    genP->description     = &param.second;
    genP->value           = genP->description->defaultVal;
    m_params[param.first] = genP;
    registerParam(param.first, genP);
  }
}

void
GenericCompositeModel::loadScripts()
{
  auto &scripts = m_recipe->scripts();

  for (auto &p : scripts)
    if (!loadScript(p))
      throw std::runtime_error("Failed to load Python script `" + p + "'");
}

GenericComponentParamEvaluator *
GenericCompositeModel::makeExpression(
  std::string const &expr,
  GenericEvaluatorSymbolDict *dict)
{
  GenericComponentParamEvaluator *paramEvaluator = new GenericComponentParamEvaluator();
  std::list<GenericCustomFunction *> customFuncs;

#ifdef PYTHON_SCRIPT_SUPPORT
  for (auto p : m_scripts) {
    auto &funcs = p->customFunctions();

    for (auto &q : funcs)
      customFuncs.push_back(static_cast<GenericCustomFunction *>(&q));
  }
#endif // PYTHON_SCRIPT_SUPPORT

  if (expr[0] == '"' && expr.size() >= 2) {
    // Assign string. Nothing 
    std::string assignStr = expr.substr(1, expr.size() - 2);
    paramEvaluator->assignString = assignStr;
    paramEvaluator->evaluator = nullptr;
    m_expressions.push_back(paramEvaluator);
  } else {
    auto evaluator = allocateEvaluator(expr, dict, customFuncs, randState());

    paramEvaluator->evaluator = evaluator;
    
    m_expressions.push_back(paramEvaluator);

    // Update dependencies. For each parameter or variable this depends on,
    // we push the new expression as a dependency. Each time any of these
    // parameters is changed, the expression must be re-evaluated.
    auto deps = evaluator->dependencies();

    for (auto dep : deps) {
      auto it = dict->find(dep);
      if (it != dict->end())
        (*dict)[dep]->dependencies.push_back(paramEvaluator);
    }
  }

  return paramEvaluator;
}       

void
GenericCompositeModel::createLocalExpressions(
    GenericEvaluatorSymbolDict &prev,
    RecipeContext *localFrame)
{
  GenericEvaluatorSymbolDict local = prev;

  // For each local frame, we need to do:
  //  1. Make expressions for the parameters of this context
  //  2. Add these parameters to the local symtab
  //  3. Create expressions for local variables and add them to local symtab
  //  4. Make expressions for the element parameters inside this context

  std::string globalPrefix = m_prefix + localFrame->currNS();

  ///////////////////////// CREATE FRAME EXPRESSIONS ///////////////////////////
  // Make them and expose their storage.
  for (auto p : localFrame->params) {
    auto name     = p.first;
    auto genP     = allocateParam();
    genP->value   = 0;

    local[name]   = genP;

    auto expr = makeExpression(p.second->expression, &local);

    expr->description = p.second;
    expr->storage     = genP;

    switch (localFrame->type) {
      case RECIPE_CONTEXT_TYPE_ROOT:
        throw std::runtime_error("Root context must have no params");
        break;

      case RECIPE_CONTEXT_TYPE_PORT:
        throw std::runtime_error("Port contexts must have no params");
        break;

      case RECIPE_CONTEXT_TYPE_ROTATION:
        expr->type = GENERIC_MODEL_PARAM_TYPE_ROTATED_FRAME;
        expr->rotation = static_cast<RotatedFrame *>(m_frames[localFrame->s_index]);
        break;

      case RECIPE_CONTEXT_TYPE_TRANSLATION:
        expr->type = GENERIC_MODEL_PARAM_TYPE_TRANSLATED_FRAME;
        expr->translation = static_cast<TranslatedFrame *>(m_frames[localFrame->s_index]);
        break;
    }
  }
  
  /////////////////////////// CREATE LOCAL VARIABLES ///////////////////////////
  // This are created to abbreviate certain expressions used along the model
  for (auto name : localFrame->varNames) {
    auto p            = localFrame->variables.find(name);
    auto genP         = allocateParam();

    genP->value       = 0;

    local[name]       = genP;
    auto expr         = makeExpression(p->second->expression, &local);
    expr->type        = GENERIC_MODEL_PARAM_TYPE_VARIABLE;
    expr->description = p->second;
    expr->storage     = genP;
  }

  /////////////////////////// CREATE PARAM EXPRESSIONS /////////////////////////
  // Parameters are set but not stored anywhere.
  for (auto elem : localFrame->elements) {
    unsigned int count = 0;

    for (auto p : elem->positionalParams) {
      auto expr         = makeExpression(p->expression, &local);
      expr->type        = GENERIC_MODEL_PARAM_TYPE_ELEMENT;
      expr->description = p;
      expr->position    = count++;
      expr->element     = m_elements[expr->description->s_target];
    }

    for (auto p : elem->params) {
      auto expr         = makeExpression(p.second->expression, &local);
      expr->type        = GENERIC_MODEL_PARAM_TYPE_ELEMENT;
      expr->description = p.second;
      expr->element     = m_elements[expr->description->s_target];
    }
  }

  for (auto ctx : localFrame->contexts)
    createLocalExpressions(local, ctx);
}

GenericEvaluatorSymbolDict const &
GenericCompositeModel::symbolDict() const
{
  return m_global;
}

ExprRandomState *
GenericCompositeModel::randState() const
{
  return m_randState;
}

void
GenericCompositeModel::initGlobalScope()
{
  // Start creation of global symbol table
  if (m_parentModel != nullptr) {
    auto parentScope = m_parentModel->symbolDict();
    for (auto p : parentScope)
      m_global[p.first] = p.second;
  }

  // Elements of the symbol table related to the DOFs and parameters
  for (auto p : m_params) {
    auto name = m_prefix + p.first;
    auto mPar = p.second;

    m_global[name] = mPar;
  }

  for (auto p : m_dofs) {
    auto name = m_prefix + p.first;
    auto mPar = p.second;

    m_global[name] = mPar;
  }
}

void
GenericCompositeModel::createExpressions()
{
  // Create local expressions recursively
  createLocalExpressions(m_global, m_recipe->rootContext());
}

bool
GenericCompositeModel::getLastDottedElement(
  std::string const &fullName,
  std::string &prefix,
  std::string &suffix)
{
  auto index = fullName.find_last_of('.');

  if (index == std::string::npos)
    return false;

  if (index == 0 || index == fullName.size() - 1)
    return false;

  prefix = fullName.substr(0, index);
  suffix = fullName.substr(index + 1);

  return true;
}


void
GenericCompositeModel::exposeOpticalPaths()
{
  for (auto path : m_recipe->paths())
    m_model->addOpticalPath(path->name, path->steps);
}

void
GenericCompositeModel::exposePorts()
{
  for (auto port : m_recipe->ports()) {
    ReferenceFrame *frame = getFrameOfContext(port.second);
    exposePort(port.first, frame);
  }
}

void
GenericCompositeModel::exposePort(std::string const &, ReferenceFrame *)
{
  // NO-OP
}
