#include "SimpleExpressionEvaluator.h"
#include <exprtk.hpp>

typedef exprtk::symbol_table<RZ::Real> exprtk_symtab_t;
typedef exprtk::expression<RZ::Real>   exprtk_expr_t;
typedef exprtk::parser<RZ::Real>       exprtk_parser_t;
typedef typename exprtk_parser_t::dependent_entity_collector::symbol_t exprtk_symbol_t;

////////////////////////////// ExprTkEvaluatorImpl /////////////////////////////
class SimpleExpressionEvaluatorImpl {
  exprtk_symtab_t        m_symTab;
  exprtk_expr_t          m_expr;
  exprtk_parser_t        m_parser;

public:
  SimpleExpressionEvaluatorImpl(
      SimpleExpressionDict const &);

  bool compile(std::string const &name);
  RZ::Real evaluate();
  std::list<std::string> const &dependencies() const;
  std::string getLastParserError() const;
};

SimpleExpressionEvaluatorImpl::SimpleExpressionEvaluatorImpl(
  SimpleExpressionDict const &dict)
{
  for (auto p : dict)
    m_symTab.add_variable(p.first, *p.second);

  m_expr.register_symbol_table(m_symTab);

  // To retrieve the symbol list
  m_parser.dec().collect_variables() = true;
}

bool
SimpleExpressionEvaluatorImpl::compile(std::string const &expr)
{
  bool ret = m_parser.compile(expr, m_expr);

  if (ret) {
    std::deque<exprtk_symbol_t> symbol_list;
    m_parser.dec().symbols(symbol_list);
  }

  return ret;
}

RZ::Real
SimpleExpressionEvaluatorImpl::evaluate()
{
  return m_expr.value();
}

std::string
SimpleExpressionEvaluatorImpl::getLastParserError() const
{
  return m_parser.error();
}



//////////////////////////// ExprTkEvaluator ///////////////////////////////////
SimpleExpressionEvaluator::SimpleExpressionEvaluator(SimpleExpressionDict const &dict)
{
  p_impl = new SimpleExpressionEvaluatorImpl(dict);
}

SimpleExpressionEvaluator::~SimpleExpressionEvaluator()
{
  delete p_impl;
}

bool
SimpleExpressionEvaluator::compile(std::string const &expr)
{
  return p_impl->compile(expr);
}

RZ::Real
SimpleExpressionEvaluator::evaluate()
{
  return p_impl->evaluate();
}

std::string
SimpleExpressionEvaluator::getLastParserError() const
{
  return p_impl->getLastParserError();
}
