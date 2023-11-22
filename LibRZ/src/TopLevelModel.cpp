#include <TopLevelModel.h>
#include <ExprTkEvaluator.h>
#include <ApertureStop.h>
#include <Recipe.h>
#include <ParserContext.h>
#include <Helpers.h>

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


void
TopLevelModel::notifyDetector(
  std::string const &preferredName,
  Detector *det)
{
  registerDetectorAlias(preferredName, det);
}

GenericEvaluator *
TopLevelModel::allocateEvaluator(
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

TopLevelModel *
TopLevelModel::fromFile(
  std::string const &path,
  std::list<std::string> const &searchPaths)
{
  std::string exceptionString;
  TopLevelModel *tlModel = nullptr;

  Recipe *recipe = new Recipe();
  recipe->addDof("t", 0, 0, 1e6);

  FileParserContext *ctx = new FileParserContext(recipe);

  for (auto &srchPath : searchPaths)
    ctx->addSearchPath(srchPath);

  FILE *fp = fopen(path.c_str(), "r");
  
  if (fp == nullptr) {
    exceptionString = string_printf(
      "Cannot open %s: %s\n",
      path.c_str(),
      strerror(errno));
    goto done;
  }

  ctx->setFile(fp, path);
  fp = nullptr;

  if (!ctx->parse()) {
    exceptionString = string_printf(
      "Cannot load %s: model has errors\n",
      path.c_str());
    goto done;
  }

  try {
    tlModel = new TopLevelModel(recipe);
  } catch (std::runtime_error const &e) {
    exceptionString = e.what();
  }

done:
  if (fp != nullptr)
    fclose(fp);

  delete ctx;

  if (tlModel == nullptr)
    throw std::runtime_error(exceptionString);

  return tlModel;
}
