#include "ExprTkEvaluator.h"
#include "exprtk.hpp"

typedef exprtk::symbol_table<RZ::Real> exprtk_symtab_t;
typedef exprtk::expression<RZ::Real>   exprtk_expr_t;
typedef exprtk::parser<RZ::Real>       exprtk_parser_t;
typedef typename exprtk_parser_t::dependent_entity_collector::symbol_t exprtk_symbol_t;

////////////////////////////// ExprTkEvaluatorImpl /////////////////////////////
namespace RZ {
  class ExprTkEvaluatorImpl {
    exprtk_symtab_t        m_symTab;
    exprtk_expr_t          m_expr;
    exprtk_parser_t        m_parser;
    std::list<std::string> m_deps;

  public:
    ExprTkEvaluatorImpl(
      GenericEvaluatorSymbolDict *dict);
    virtual ~ExprTkEvaluatorImpl();

    bool compile(std::string const &name);
    Real evaluate();
    std::list<std::string> const &dependencies() const;
    std::string getLastParserError() const;
  };
}

using namespace RZ;

ExprTkEvaluatorImpl::ExprTkEvaluatorImpl(
  GenericEvaluatorSymbolDict *dict)
{
  for (auto p : *dict)
    m_symTab.add_variable(p.first, p.second->value);

  m_expr.register_symbol_table(m_symTab);

  // To retrieve the symbol list
  m_parser.dec().collect_variables() = true;
}

ExprTkEvaluatorImpl::~ExprTkEvaluatorImpl()
{
}

bool
ExprTkEvaluatorImpl::compile(std::string const &expr)
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
ExprTkEvaluatorImpl::evaluate()
{
  return m_expr.value();
}

std::string
ExprTkEvaluatorImpl::getLastParserError() const
{
  return m_parser.error();
}

std::list<std::string> const &
ExprTkEvaluatorImpl::dependencies() const
{
  return m_deps;
}

//////////////////////////// ExprTkEvaluator ///////////////////////////////////
ExprTkEvaluator::ExprTkEvaluator(GenericEvaluatorSymbolDict *dict) :
  GenericEvaluator(dict)
{
  p_impl = new ExprTkEvaluatorImpl(dict);
}
ExprTkEvaluator::~ExprTkEvaluator()
{
  delete p_impl;
}

std::list<std::string>
ExprTkEvaluator::dependencies() const
{
  return p_impl->dependencies();
}

bool
ExprTkEvaluator::compile(std::string const &expr)
{
  return p_impl->compile(expr);
}

Real
ExprTkEvaluator::evaluate()
{
  return p_impl->evaluate();
}

std::string
ExprTkEvaluator::getLastParserError() const
{
  return p_impl->getLastParserError();
}
