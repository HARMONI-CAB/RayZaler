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
  registerProperty(name, param->value);
}

void
CompositeElement::registerParam(
  std::string const &name, 
  GenericModelParam *param)
{
  registerProperty(name, param->value);
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
  GenericEvaluatorSymbolDict *dict,
  std::list<GenericCustomFunction *> const &functions,
  ExprRandomState *state)
{
  ExprTkEvaluator *eval = new ExprTkEvaluator(dict, state);

  for (auto p : functions)
    eval->registerCustomFunction(p);

  if (!eval->compile(expr)) {
    std::string error = eval->getLastParserError();
    delete eval;

    throw std::runtime_error("Expression error: " + error);
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

  setRandomState(parentCompositeModel->randState());

  build(pFrame);
}

OpticalPath
CompositeElement::opticalPath() const
{
  const OpticalPath *path = m_model->lookupOpticalPath();

  if (path == nullptr)
    throw std::runtime_error("Element `" + name() + "' did not expose a default optical path");
  
  return *path;
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

CompositeElement::~CompositeElement()
{
  delete m_model;
}

/////////////////////////// CompositeElementFactory ////////////////////////////
CompositeElementFactory::CompositeElementFactory(
  std::string const &name,
  Recipe *recipe,
  GenericCompositeModel *owner)
  : m_name(name), m_recipe(recipe), m_owner(owner)
{
}

std::string
CompositeElementFactory::name() const
{
  return m_name;
}

Element *
CompositeElementFactory::make(
    std::string const &name,
    ReferenceFrame *pFrame,
    Element *parent)
{
  return new CompositeElement(this, name, pFrame, m_recipe, m_owner, parent);
}
