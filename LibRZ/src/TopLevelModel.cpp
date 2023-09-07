#include <TopLevelModel.h>
#include "ExprTkEvaluator.h"
#include "ApertureStop.h"

using namespace RZ;

TopLevelModel::TopLevelModel(Recipe *recipe) :
  OMModel(),
  GenericCompositeModel(recipe, this)
{
  build(world());
}

TopLevelModel::~TopLevelModel()
{
  
}

void
TopLevelModel::registerDof(std::string const &name, GenericModelParam *param)
{
  // NO-OP
}

void
TopLevelModel::registerParam(std::string const &name, GenericModelParam *param)
{
  // NO-OP
}

void
TopLevelModel::exposePort(std::string const &name, ReferenceFrame *frame)
{
  m_focalPlanes[name] = frame;
}

std::list<std::string>
TopLevelModel::focalPlanes() const
{
  std::list<std::string> planes;

  for (auto p : m_focalPlanes)
    planes.push_back(p.first);
  
  return planes;
}

std::list<std::string>
TopLevelModel::apertureStops() const
{
  std::list<std::string> stops;

  auto elements = opticalElements();
  for (auto p : elements) {
    auto element = lookupOpticalElement(p);

    if (element->factory()->name() == "ApertureStop")
      stops.push_back(p);
  }

  return stops;
}

ApertureStop *
TopLevelModel::getApertureStop(std::string const &name) const
{
  auto element = lookupOpticalElement(name);

  if (element == nullptr)
    return nullptr;

  if (element->factory()->name() != "ApertureStop")
    return nullptr;

  return static_cast<ApertureStop *>(element);
}

ReferenceFrame *
TopLevelModel::getFocalPlane(std::string const &name) const
{
  auto p = m_focalPlanes.find(name);

  if (p == m_focalPlanes.end())
    return nullptr;

  return p->second;
}

void
TopLevelModel::registerOpticalPath(
  std::string const &name,
  std::list<std::string> &steps)
{
  if (!addOpticalPath(name, steps))
    throw std::runtime_error("Failed to register optical path `" + name + "'");
}

GenericEvaluator *
TopLevelModel::allocateEvaluator(
  std::string const &expr,
  GenericEvaluatorSymbolDict *dict)
{
  ExprTkEvaluator *eval = new ExprTkEvaluator(dict);

  if (!eval->compile(expr)) {
    std::string error = eval->getLastParserError();
    delete eval;

    throw std::runtime_error("Expression error: " + error);
  }

  return eval;
}
