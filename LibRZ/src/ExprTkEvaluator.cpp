#include "ExprTkEvaluator.h"

#define exprtk_disable_caseinsensitivity
#include <exprtk.hpp>
#include <random>

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

#define MERSENNE_TWISTER_DISCARD 5

struct randu : public exprtk::ifunction<Real> {
  std::mt19937_64                      m_generator;
  std::uniform_real_distribution<Real> m_dist;

  randu() : exprtk::ifunction<RZ::Real>(2)
  {
    m_dist = std::uniform_real_distribution<Real>(0, 1);
  }

  Real
  operator()(Real const &seed, Real const &index)
  {
    m_generator.seed(static_cast<int>(seed) + static_cast<int>(index));
    m_generator.discard(MERSENNE_TWISTER_DISCARD);
    m_dist.reset();
    
    return m_dist(m_generator);
  }
};

struct randn : public exprtk::ifunction<Real> {
  std::mt19937_64                m_generator;
  std::normal_distribution<Real> m_dist;

  RZ::Real m_lastSeed = 1;

  randn() : exprtk::ifunction<RZ::Real>(2)
  {
    m_dist = std::normal_distribution<Real>(0, 1);
  }

  Real
  operator()(Real const &seed, Real const &index)
  {
    m_generator.seed(static_cast<int>(seed) + static_cast<int>(index));
    m_generator.discard(MERSENNE_TWISTER_DISCARD);
    m_dist.reset();
    return m_dist(m_generator);
  }
};

static randu g_randu;
static randn g_randn;

ExprTkEvaluatorImpl::ExprTkEvaluatorImpl(
  GenericEvaluatorSymbolDict *dict)
{
  for (auto p : *dict)
    m_symTab.add_variable(p.first, p.second->value);

  m_symTab.add_function("randu", g_randu);
  m_symTab.add_function("randn", g_randn);
  
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
