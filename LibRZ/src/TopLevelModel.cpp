#include <TopLevelModel.h>
#include "ExprTkEvaluator.h"

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
