#include <GenericCompositeModel.h>
#include <Recipe.h>
#include <RotatedFrame.h>
#include <TranslatedFrame.h>
#include <Singleton.h>
#include <OMModel.h>

using namespace RZ;

std::list<std::string>
GenericEvaluator::symbols()
{
  std::list<std::string> list;

  for (auto p : *m_dict)
    list.push_back(p.first);

  return list;
}

Real *
GenericEvaluator::resolve(std::string const &symbol)
{
  auto it = m_dict->find(symbol);

  if (it != m_dict->end())
    return &(*m_dict)[symbol]->value;

  return nullptr;
}

GenericEvaluator::GenericEvaluator(GenericEvaluatorSymbolDict *dict)
{
  m_dict = dict;
}

GenericEvaluator::~GenericEvaluator()
{
}

GenericComponentParamEvaluator::~GenericComponentParamEvaluator()
{
  if (evaluator != nullptr)
    delete evaluator;
}

void
GenericComponentParamEvaluator::assign()
{
  std::string param = description->parameter;
  Real value;

  if (description == nullptr)
    throw std::runtime_error("Param evaluator has no description");

  if (description->s_target == -1)
    throw std::runtime_error("Param evaluator has undefined object index");

  // Hey, butler, stop butling yourself!
  value = evaluator->evaluate();

  switch (type) {
    case GENERIC_MODEL_PARAM_TYPE_ELEMENT:
      element->set(param, value);
      break;

    case GENERIC_MODEL_PARAM_TYPE_ROTATED_FRAME:
      if (param == "angle")
        rotation->setAngle(value);
      else if (param == "eX")
        rotation->setAxisX(value);
      else if (param == "eY")
        rotation->setAxisY(value);
      else if (param == "eZ")
        rotation->setAxisZ(value);
      else
        std::runtime_error("Unknown rotation parameter `" + param + "'");
      
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
        std::runtime_error("Unknown translation parameter `" + param + "'");

      translation->recalculate();
      break;
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
  Element *parent)
{
  m_recipe = recipe;
  m_model  = model;
  m_parent = parent;
}

GenericCompositeModel::~GenericCompositeModel()
{
  for (auto p : m_expressions)
    delete p;

  for (auto p : m_genParamStorage)
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

void
GenericCompositeModel::assignEverything()
{
  for (auto p : m_expressions)
    p->assign();
}

bool
GenericCompositeModel::setParam(std::string const &name, Real value)
{
  GenericModelParam *param = lookupParam(name);

  if (param == nullptr)
    throw std::runtime_error("Unknown parameter `" + name + "'");

  if (param->test(value))
    return false;

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

  if (!dof->test(value))
    return false;

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

  createFrames(parent);
  createElements(parent);

  delayedCreationLoop();

  createParams();
  createExpressions();
  exposeOpticalPaths();

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
        std::runtime_error("Root context in wrong place");
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
      std::runtime_error("Reference frame `" + name + "' already exists");
  }
}

void
GenericCompositeModel::createElementInside(
  RecipeElementStep *step,
  ReferenceFrame *pFrame)
{
  std::string name = m_prefix + step->name;
  Singleton *sing = Singleton::instance();
  int index = step->s_index;

  auto factory = sing->lookupElementFactory(step->factory);
  if (factory == nullptr)
    std::runtime_error("Undefined element class `" + step->factory + "'");

  m_elements[index] = factory->make(name, pFrame, m_parent);
  ++m_completedElements;

  if (!m_model->autoRegisterElement(m_elements[index]))
    std::runtime_error("Element`" + name + "' already exists");
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

GenericComponentParamEvaluator *
GenericCompositeModel::makeExpression(
  std::string const &expr,
  GenericEvaluatorSymbolDict *dict)
{
  GenericComponentParamEvaluator *paramEvaluator = new GenericComponentParamEvaluator();

  auto evaluator = allocateEvaluator(expr, dict);
  
  paramEvaluator->evaluator = evaluator;
  
  m_expressions.push_back(paramEvaluator);

  // Update dependencies
  auto deps = evaluator->dependencies();

  for (auto dep : deps) {
    auto it = dict->find(dep);
    if (it != dict->end())
      (*dict)[dep]->dependencies.push_back(paramEvaluator);
  }

  return paramEvaluator;
}       

void
GenericCompositeModel::createLocalExpressions(
    GenericEvaluatorSymbolDict &global,
    RecipeContext *localFrame)
{
  GenericEvaluatorSymbolDict local = global;

  // For each local frame, we need to do:
  //  1. Create parameters for element and frame parameters
  //  2. Populate the local symtab
  //  3. Make expressions

  std::string globalPrefix = m_prefix + localFrame->currNS();

  ///////////////////////////// SYMTAB CREATION //////////////////////////////
  // Expose frame parameters
  for (auto p : localFrame->params) {
    auto name        = p.second->parameter;
    auto fullName    = globalPrefix + "." + name;
    
    auto genP        = allocateParam();
    genP->value      = 0;
    
    global[fullName] = genP;
    local[name]      = genP;
  }

  // Expose element parameters
  for (auto elem : localFrame->elements) {
    for (auto p : elem->params) {
      auto name     = elem->name + "." + p.second->parameter;
      auto fullName = globalPrefix + "." + name;
      
      auto genP     = allocateParam();
      genP->value   = 0;

      global[fullName] = genP;
      local[name]      = genP;
    }
  }

  /////////////////////////// EXPRESSION CREATION ////////////////////////////
  // Make expressions
  for (auto elem : localFrame->elements) {
    for (auto p : elem->params) {
      auto expr         = makeExpression(p.second->expression, &local);
      expr->type        = GENERIC_MODEL_PARAM_TYPE_ELEMENT;
      expr->description = p.second;
      expr->element     = m_elements[expr->description->s_target];
    }
  }

  for (auto p : localFrame->params) {
    auto expr = makeExpression(p.second->expression, &local);
    expr->description = p.second;

    switch (localFrame->type) {
      case RECIPE_CONTEXT_TYPE_ROOT:
        std::runtime_error("Root context must have no params");
        break;

      case RECIPE_CONTEXT_TYPE_PORT:
        std::runtime_error("Port contexts must have no params");
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
}

void
GenericCompositeModel::createExpressions()
{
  // Start creation of global symbol table
  GenericEvaluatorSymbolDict global;
  auto ctx = m_recipe->contexts();

  // Elements of the symbol table related to the DOFs and parameters
  for (auto p : m_params) {
    auto name = m_prefix + p.first;
    auto mPar = p.second;

    global[name] = mPar;
  }

  for (auto p : m_dofs) {
    auto name = m_prefix + p.first;
    auto mPar = p.second;

    global[name] = mPar;
  }

  // Context by context, create expressions
  for (auto p : ctx)
    createLocalExpressions(global, p);
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

}
