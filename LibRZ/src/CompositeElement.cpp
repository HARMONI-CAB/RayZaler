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

#include "CompositeElement.h"
#include "OMModel.h"
#include "ExprTkEvaluator.h"
#include "Logger.h"

using namespace RZ;

void
CompositeElement::registerDof(
  std::string const &name, 
  GenericModelParam *param)
{
  registerProperty(name, param->value, "Degree of freedom (" + name + ")");
}

void
CompositeElement::registerParam(
  std::string const &name, 
  GenericModelParam *param)
{
  registerProperty(name, param->value, "Model parameter (" + name + ")");
}

void
CompositeElement::registerOpticalPath(
  std::string const &name,
  std::list<std::string> &params)
{
  m_model->addOpticalPath(name, params);
}

GenericEvaluator *
CompositeElement::allocateEvaluator(
  std::string const &expr,
  const GenericEvaluatorSymbolDict *dict,
  std::list<GenericCustomFunction *> const &functions,
  ExprRandomState *state)
{
  ExprTkEvaluator *eval = new ExprTkEvaluator(dict, state);

  for (auto p : functions)
    eval->registerCustomFunction(p);

  if (!eval->compile(expr)) {
    std::string error = eval->getLastParserError();
    delete eval;
    throw std::runtime_error("Expression error: " + error + " in expression `" + expr + "` (element " + name() + ")");
  }

  return eval;
}

void
CompositeElement::exposePort(
  std::string const &name,
  ReferenceFrame *frame)
{
  // Ports are ADDED, because they belong to the underlying OMModel.
  if (!addPort(name, frame))
    throw std::runtime_error("Internal error: port `" + name + "' redefined");

  m_model->setFrameAlias(frame, name);
}

bool
CompositeElement::propertyChanged(
  std::string const &name,
  PropertyValue const &val)
{
  Real value = val;

  try {
    setDof(name, value);
    return true;
  } catch (std::runtime_error &e) { }

  try {
    setParam(name, value);
    return true;
  } catch (std::runtime_error &e) { }
  
  if (!Element::propertyChanged(name, val)) {
    RZError("Uknown property`%s'", name.c_str());
    return false;
  }

  return true;
}

void
CompositeElement::notifyDetector(
  std::string const &preferredName,
  Detector *det)
{
  GenericCompositeModel *parent = parentCompositeModel();

  if (parent != nullptr)
    parent->notifyDetector(name() + "." + preferredName, det);
}

IHateCPlusPlus::IHateCPlusPlus(OMModel *model)
{
  m_model = model;
}

CompositeElement::CompositeElement(
  ElementFactory *factory,
  std::string const &name, 
  ReferenceFrame *pFrame,
  Recipe *recipe,
  GenericCompositeModel *parentCompositeModel,
  Element *parent) : 
  IHateCPlusPlus(new OMModel()),
  OpticalElement(factory, name, pFrame, parent),
  GenericCompositeModel(recipe, model(), parentCompositeModel)
{
  m_model = model();
  m_model->linkWorld(pFrame);
  
  setName(name);

  setRandomState(parentCompositeModel->randState());

  build(pFrame);
}

OpticalPath
CompositeElement::opticalPath(std::string const &pathName) const
{
  const OpticalPath *path = m_model->lookupOpticalPath(pathName);

  if (path == nullptr)
    if (pathName.empty())
      throw std::runtime_error("Element `" + name() + "' did not expose a default optical path");
    else
      throw std::runtime_error("Element `" + name() + "' does not have an optical path named `" + pathName + "'");
  
  return *path;
}

void
CompositeElement::setRecordHits(bool record)
{
  for (auto p : m_model->opticalElements()) {
    auto element = m_model->lookupOpticalElement(p);
    element->setRecordHits(record);
  }   
}

void
CompositeElement::clearHits()
{
  for (auto p : m_model->opticalElements()) {
    auto element = m_model->lookupOpticalElement(p);
    element->clearHits();
  }
}

void
CompositeElement::renderOpenGL()
{
  // NO-OP (but it will be a bounding box)
}

OMModel *
CompositeElement::nestedModel() const
{
  return m_model;
}

GenericCompositeModel *
CompositeElement::nestedCompositeModel() const
{
  // C++. The ceiling has been raised.
  
  return const_cast<GenericCompositeModel *>(
    static_cast<const GenericCompositeModel *>(
      this));
}

CompositeElement::~CompositeElement()
{
  delete m_model;
}

/////////////////////////// CompositeElementFactory ////////////////////////////
CompositeElementFactory::CompositeElementFactory(
  std::string const &name,
  Recipe *recipe,
  GenericCompositeModel *owner) : OpticalElementFactory(),
  m_name(name), m_recipe(recipe), m_owner(owner)
{
  enterDecls(name, "User-defined composite element");
}

Element *
CompositeElementFactory::make(
    std::string const &name,
    ReferenceFrame *pFrame,
    Element *parent)
{
  return new CompositeElement(this, name, pFrame, m_recipe, m_owner, parent);
}
