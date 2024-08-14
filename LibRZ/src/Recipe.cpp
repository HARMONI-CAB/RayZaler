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

#include <Recipe.h>
#include <iostream>
#include <algorithm>
#include <Singleton.h>
#include <Logger.h>
#include <libgen.h>
#include <algorithm>

using namespace RZ;

void
RecipeParamListProto::set(std::string const &name, std::string const &value)
{
  if (name.size() == 0) {
    // Positional

    if (nonPositionalNdx >= params.size())
      throw std::runtime_error("Too many parameters");
    if (nonPositionalNdx > 0)
      throw std::runtime_error("Cannot set positional parameters after key-value arguments");

    auto name = params[positionalNdx++];
    defined.push_back(name);
    values[name] = value;
  } else {
    if (std::find(params.begin(), params.end(), name) == params.end())
      throw std::runtime_error("Unknown parameter `" + name + "'");

    if (isSet(name))
      throw std::runtime_error("Parameter `" + name + "' set twice");
    
    defined.push_back(name);
    values[name] = value;
    ++nonPositionalNdx;
  }
}

bool
RecipeParamListProto::isSet(std::string const &name) const
{
  return std::find(defined.begin(), defined.end(), name) != defined.end();
}

std::string
RecipeContext::to_string() const
{
  std::string fullStr;

  fullStr = currNS() + " [";

  switch (type) {
    case RECIPE_CONTEXT_TYPE_ROOT:
      fullStr += "Root context";
      break;

    case RECIPE_CONTEXT_TYPE_ROTATION:
      fullStr += "Rotation";
      break;

    case RECIPE_CONTEXT_TYPE_TRANSLATION:
      fullStr += "Translation";
      break;

    case RECIPE_CONTEXT_TYPE_PORT:
      fullStr += "Port `" + port + "' of `" + element->name + "'";
      break;
  }

  fullStr += "]";

  if (params.size() > 0) {
    fullStr += " (";
    unsigned int count = 0;
    for (auto param : params) {
      if (count++ > 0)
        fullStr += ", ";

      fullStr += param.first + " = " + param.second->expression;
    }

    fullStr += ")";
  }

  return fullStr;
}

void
RecipeElementStep::set(std::string const &name, std::string const &expr)
{
  if (name.empty()) {
    // Positional argument. Only allowed if no keyed parameters are
    // defined.
    if (params.size() != 0)
      throw std::runtime_error("Cannot set positional parameters after key-value arguments");
  }

  auto it = params.find(name);

  if (it != params.end())
    params[name]->expression = expr;
  else
    owner->makeElementParameter(this, name, expr);
}

Recipe *
Recipe::parent() const
{
  return m_parent;
}

Recipe::Recipe(std::string const &name, Recipe *parent)
{
  // Create root context
  m_rootContext       = makeContext(nullptr);
  m_rootContext->name = name;
  m_currContext       = m_rootContext;
  m_parent            = parent;
}

Recipe::Recipe() : Recipe("world", nullptr)
{

}

Recipe::~Recipe()
{
  for (auto p : m_contexts)
    delete p;

  for (auto p : m_elementSteps)
    delete p;

  for (auto p : m_pathSteps)
    delete p;

  for (auto p : m_elemParameters)
    delete p;

  for (auto p : m_frameParameters)
    delete p;

  for (auto p : m_variables)
    delete p;
  
  for (auto p: m_subRecipes)
    delete p;
}

std::string
Recipe::currNS() const
{
  if (m_currContext == nullptr)
    return "";

  return m_currContext->currNS();
}

RecipeContext *
Recipe::lookupReferenceFrame(std::string const &name) const
{
  auto it = m_frames.find(name);

  if (it == m_frames.end())
    return nullptr;

  return it->second;
}

RecipeElementStep *
Recipe::lookupElement(std::string const &name) const
{
  auto it = m_elements.find(name);

  if (it == m_elements.end())
    return nullptr;

  return it->second;
}

Recipe *
Recipe::makeCustomElement(std::string const &name)
{
  // Make sure element has not been defined twice
  auto it = m_customElements.find(name);
  if (it != m_customElements.end())
    return nullptr;

  // Make sure we are not attempting to override an existing element factory
  Singleton *sing = Singleton::instance();
  if (sing->lookupElementFactory(name) != nullptr
    || m_customElements.find(name) != m_customElements.end())
    return nullptr;
  
  Recipe *recipe = new Recipe(name, this);
  m_customElements[name] = recipe;

  m_subRecipes.push_back(recipe);

  for (auto &p : m_searchPaths)
    recipe->pushSearchPath(p);
  
  return recipe;
}

void
Recipe::pushSearchPath(std::string const &path)
{
  if (std::find(m_searchPaths.begin(), m_searchPaths.end(), path) ==
    m_searchPaths.end())
    m_searchPaths.push_back(path);
}

bool
Recipe::addScript(std::string const &scriptPath)
{
  std::vector<char> asCString(
    scriptPath.c_str(),
    scriptPath.c_str() + scriptPath.size() + 1);
  char *dirPath = dirname(asCString.data());

  pushSearchPath(dirPath);
  m_scripts.push_back(scriptPath);

  return true;
}

RecipeElementStep *
Recipe::resolveElement(std::string const &name) const
{
  std::string fullyQualifiedName;
  
  fullyQualifiedName = m_currContext->currNS() + "." + name;

  auto element = lookupElement(fullyQualifiedName);
  if (element != nullptr)
    return element;

  return lookupElement(name);
}

std::string
Recipe::genElementName(std::string const &parent, std::string const &type)
{
  std::string hint;
  unsigned int i = 1;
  std::string prefix = parent.size() > 0 ? parent + "." : "";

  if (lookupElement(prefix + type) == nullptr)
    return type;

  do {
    hint = type + "_" + std::to_string(i++);
  } while (lookupElement(prefix + hint) != nullptr);

  return hint;
}

std::string
Recipe::genReferenceFrameName(std::string const &parent, std::string const &type)
{
  std::string hint;
  unsigned int i = 1;
  std::string prefix = parent.size() > 0 ? parent + "." : "";

  if (lookupReferenceFrame(prefix + type) == nullptr)
    return type;

  do {
    hint = type + "_" + std::to_string(i++);
  } while (lookupReferenceFrame(prefix + hint) != nullptr);

  return hint;
}

std::string
Recipe::genElementName(std::string const &type)
{
  return genElementName(m_currContext->currNS(), type);
}

std::string
Recipe::genReferenceFrameName(std::string const &type)
{
  return genReferenceFrameName(m_currContext->currNS(), type);
}

RecipeContext *
Recipe::makeContext(RecipeContext *parent)
{
  int index = static_cast<int>(m_contexts.size());

  m_contexts.resize(index + 1);

  auto newCtx = new RecipeContext();
  m_contexts[index] = newCtx;
  
  newCtx->s_index = index;
  newCtx->parent = parent;

  // Keep track of the current namespace and the context hierarchy
  if (parent != nullptr) {
    newCtx->parentNS = parent->currNS();
    parent->contexts.push_back(newCtx);
  }

  return newCtx;
}

RecipeElementStep *
Recipe::makeElementStep(RecipeContext *parent)
{
  int index = static_cast<int>(m_elementSteps.size());
  m_elementSteps.resize(index + 1);

  m_elementSteps[index] = new RecipeElementStep();
  m_elementSteps[index]->s_index = index;
  m_elementSteps[index]->parent = parent;
  m_elementSteps[index]->owner = this;
  m_elementSteps[index]->delayedCreation = m_nestedPorts > 0;

  return m_elementSteps[index];
}

RecipeOpticalPath *
Recipe::makeOpticalPathStep(RecipeContext *parent)
{
  int index = static_cast<int>(m_pathSteps.size());
  m_pathSteps.resize(index + 1);

  m_pathSteps[index] = new RecipeOpticalPath();
  m_pathSteps[index]->parent = parent;

  return m_pathSteps[index];
}

RecipeParameter *
Recipe::makeRecipeParam(
  std::map<std::string, RecipeParameter> &dest,
  std::string const &name)
{
  auto it = dest.find(name);

  if (it != dest.cend())
    return nullptr;

  dest[name] = RecipeParameter();

  return &dest[name];
}

ParamAssignExpression *
Recipe::makeElementParameter(
        RecipeElementStep *elem,
        std::string const &name,
        std::string const &expression)
{
  int index = static_cast<int>(m_elemParameters.size());
  m_elemParameters.resize(index + 1);

  auto curr        = new ParamAssignExpression();
  curr->parameter  = name;
  curr->expression = expression;
  curr->parent     = m_currContext;
  curr->s_index    = index;

  curr->s_target   = elem->s_index;

  m_elemParameters[index] = curr;

  if (name.empty())
    elem->positionalParams.push_back(curr);
  else
    elem->params[name] = curr;

  return curr;
}
      
ParamAssignExpression *
Recipe::makeReferenceFrameParameter(
        RecipeContext *ctx,
        std::string const &name,
        std::string const &expression)
{
  int index = static_cast<int>(m_frameParameters.size());
  m_frameParameters.resize(index + 1);

  auto curr        = new ParamAssignExpression();
  curr->parameter  = name;
  curr->expression = expression;
  curr->parent     = m_currContext;
  curr->s_index    = index;
  curr->s_target   = ctx->s_index;

  m_frameParameters[index] = curr;
  ctx->params[name]        = curr;
  
  return curr;
}

ParamAssignExpression *
Recipe::makeVariable(
        RecipeContext *ctx,
        std::string const &name,
        std::string const &expression)
{
  int index = static_cast<int>(m_variables.size());
  m_variables.resize(index + 1);

  auto curr = new ParamAssignExpression();
  curr->parameter  = name;
  curr->expression = expression;
  curr->parent     = m_currContext;
  curr->s_index    = index;
  curr->s_target   = ctx->s_index;

  m_variables[index]   = curr;
  ctx->variables[name] = curr;
  ctx->varNames.push_back(name);
  
  return curr;
}

void
Recipe::push(RecipeContext *ctx)
{
  // It has a name! Expose it.
  if (ctx->name.size() > 0) {
    std::string fullQualifiedName = ctx->currNS();
    auto p = lookupReferenceFrame(fullQualifiedName);

    if (p != nullptr)
      throw std::runtime_error("Reference frame `" + fullQualifiedName + "' already exists");

    m_frames[fullQualifiedName] = ctx;
  }

  m_currContext = ctx;
}

RecipeContext *
Recipe::rootContext() const
{
  return m_rootContext;
}

std::vector<RecipeContext *> const &
Recipe::contexts() const
{
  return m_contexts;
}

std::vector<RecipeElementStep *> const &
Recipe::elements() const
{
  return m_elementSteps;
}

std::vector<RecipeOpticalPath *> const &
Recipe::paths() const
{
  return m_pathSteps;
}

void
Recipe::pushRotation(
  std::string const &angle,
  std::string const &eX,
  std::string const &eY,
  std::string const &eZ,
  std::string const &name)
{
  RecipeContext *context = makeContext(m_currContext);
  
  context->type      = RECIPE_CONTEXT_TYPE_ROTATION;
  context->name      = name.size() == 0 ? genReferenceFrameName("rot") : name;
  context->delayed   = m_nestedPorts > 0;

  makeReferenceFrameParameter(context, "angle", angle);
  makeReferenceFrameParameter(context, "eX",    eX);
  makeReferenceFrameParameter(context, "eY",    eY);
  makeReferenceFrameParameter(context, "eZ",    eZ);

  push(context);
}

void
Recipe::pushVariable(std::string const &name, std::string const &value)
{
  if (m_currContext->variables.find(name) != m_currContext->variables.end())
    throw std::runtime_error(
      "Variable `" + name + "' redefined in this context\n");

  makeVariable(m_currContext, name, value);
}

void
Recipe::pushTranslation(
  std::string const &dX,
  std::string const &dY,
  std::string const &dZ,
  std::string const &name)
{
  RecipeContext *context = makeContext(m_currContext);

  context->type      = RECIPE_CONTEXT_TYPE_TRANSLATION;
  context->name      = name.size() == 0 ? genReferenceFrameName("move") : name;
  context->delayed   = m_nestedPorts > 0;

  makeReferenceFrameParameter(context, "dX",  dX);
  makeReferenceFrameParameter(context, "dY",  dY);
  makeReferenceFrameParameter(context, "dZ",  dZ);

  push(context);
}

void
Recipe::pushPortContext(RecipeElementStep *step, std::string const &port)
{
  RecipeContext *context = makeContext(m_currContext);

  context->type    = RECIPE_CONTEXT_TYPE_PORT;
  context->name    = genReferenceFrameName(step->name + "_" + port);
  context->port    = port;
  context->element = step;

  ++m_nestedPorts;

  push(context);
}


bool
Recipe::pop()
{
  if (m_currContext == m_rootContext)
    return false;

  if (m_currContext->type == RECIPE_CONTEXT_TYPE_PORT)
    --m_nestedPorts;
  
  m_currContext = m_currContext->parent;
  return true;
}

RecipeOpticalPath *
Recipe::lookupOpticalPath(std::string const &name) const
{
  auto it = m_paths.find(name);
  if (it == m_paths.end())
    return nullptr;

  return it->second;
}

RecipeElementStep *
Recipe::addElement(
  std::string const &name,
  std::string const &factory,
  std::map<std::string, std::string> const &parameters)
{
  std::string fullyQualifiedName;
  
  if (name.empty()) {
    fullyQualifiedName = m_currContext->currNS() + "." + name;
  } else {
    // Generate new name
    unsigned int i = 0;
    do {
      std::string newName = "anonymous_" + factory + "_" + std::to_string(++i);
      fullyQualifiedName = m_currContext->currNS() + "." + newName;
    } while (m_elements.find(fullyQualifiedName) != m_elements.end());
  }


  if (lookupElement(fullyQualifiedName) != nullptr)
    throw std::runtime_error("Element `" + fullyQualifiedName + "' redefined");

  RecipeElementStep *step = makeElementStep(m_currContext);

  step->name    = name;
  step->factory = factory;

  for (auto p : parameters) {
    if (step->params.find(p.first) != step->params.end()) {
      throw std::runtime_error(
        "Element parameter `" + p.first + "' of `" + fullyQualifiedName + "' defined twice");

      makeElementParameter(step, p.first, p.second);
    }
  }

  m_elements[fullyQualifiedName] = step;

  if (!name.empty())
    m_elements[name] = step;

  m_currContext->elements.push_back(step);

  
  return step;
}

RecipeOpticalPath *
Recipe::allocatePath(std::string const &name)
{
  if (lookupOpticalPath(name) != nullptr) {
    if (name == "")
      throw std::runtime_error("Default optical path already defined");
    else
      throw std::runtime_error("Optical path `" + name + "' already defined");
  }

  RecipeOpticalPath *path = makeOpticalPathStep(m_currContext);

  path->name = name;
  m_paths[name] = path;

  return path;
}

std::map<std::string, Recipe *> const &
Recipe::customElements() const
{
  return m_customElements;
}

std::map<std::string, RecipeContext *> const &
Recipe::ports() const
{
  return m_ports;
}

std::map<std::string, RecipeParameter> const &
Recipe::dofs() const
{
  return m_dofs;
}

std::map<std::string, RecipeParameter> const &
Recipe::params() const
{
  return m_parameters;
}

void
Recipe::addPort(std::string const &name)
{
  if (m_ports.find(name) != m_ports.end())
    throw std::runtime_error("Port `" + name + "' redefined");

  m_ports[name] = m_currContext;
}

bool
Recipe::addDof(
  std::string const &name,
  Real defVal,
  Real min,
  Real max)
{
  RecipeParameter *newParam = makeRecipeParam(m_dofs, name);

  if (newParam == nullptr)
    throw std::runtime_error("Degree of freedom `" + name + "' already defined");
  
  newParam->defaultVal = defVal;
  newParam->min        = min;
  newParam->max        = max;

  return true;
}

bool
Recipe::addParam(
  std::string const &name,
  Real defVal,
  Real min,
  Real max)
{
  RecipeParameter *newParam = makeRecipeParam(m_parameters, name);

  if (newParam == nullptr)
    throw std::runtime_error("Parameter `" + name + "' already defined");
  
  newParam->defaultVal = defVal;
  newParam->min        = min;
  newParam->max        = max;

  return true; 
}

void
Recipe::debug()
{
  std::cout << "Contexts:" << std::endl;
  for (auto ctx : m_contexts) {
    std::cout << "  - " << ctx->to_string() << std::endl;
  }

  std::cout << std::endl;
  std::cout << "Elements:" << std::endl;

  for (auto el : m_elementSteps)
    std::cout << " - " << el->name << " (" << el->factory << ") in " << el->parent->currNS() << std::endl;

  std::cout << std::endl;
}
