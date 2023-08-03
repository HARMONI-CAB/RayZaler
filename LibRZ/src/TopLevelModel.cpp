#include <TopLevelModel.h>
#include "exprtk.hpp"

using namespace RZ;

typedef exprtk::symbol_table<Real> exprtk_symtab_t;
typedef exprtk::expression<Real>   exprtk_expr_t;
typedef exprtk::parser<Real>       exprtk_parser_t;
typedef typename exprtk_parser_t::dependent_entity_collector::symbol_t exprtk_symbol_t;

class TopLevelEvaluator : public GenericEvaluator {
    exprtk_symtab_t        m_symTab;
    exprtk_expr_t          m_expr;
    exprtk_parser_t        m_parser;
    std::list<std::string> m_deps;

  public:
    TopLevelEvaluator(GenericEvaluatorSymbolDict *dict);
    virtual ~TopLevelEvaluator();

    virtual bool compile(std::string const &name) override;
    virtual Real evaluate() override;
    virtual std::list<std::string> dependencies() const override;

    std::string getLastParserError() const;
};

TopLevelEvaluator::TopLevelEvaluator(GenericEvaluatorSymbolDict *dict)
: GenericEvaluator(dict)
{
  for (auto p : *dict)
    m_symTab.add_variable(p.first, p.second->value);

  m_expr.register_symbol_table(m_symTab);

  // To retrieve the symbol list
  m_parser.dec().collect_variables() = true;
}

TopLevelEvaluator::~TopLevelEvaluator()
{
}

bool
TopLevelEvaluator::compile(std::string const &expr)
{
  bool ret = m_parser.compile(expr, m_expr);

  if (ret) {
    std::deque<exprtk_symbol_t> symbol_list;
    m_parser.dec().symbols(symbol_list);

    for (auto p : symbol_list)
      m_deps.push_back(p.first);
  }

  return ret;
}

Real
TopLevelEvaluator::evaluate()
{
  return m_expr.value();
}

std::string
TopLevelEvaluator::getLastParserError() const
{
  return m_parser.error();
}

std::list<std::string>
TopLevelEvaluator::dependencies() const
{
  return m_deps;
}

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
  TopLevelEvaluator *eval = new TopLevelEvaluator(dict);

  if (!eval->compile(expr)) {
    std::string error = eval->getLastParserError();
    delete eval;

    throw std::runtime_error("Expression error: " + error);
  }

  return eval;
}
