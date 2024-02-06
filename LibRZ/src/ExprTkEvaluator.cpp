#include "ExprTkEvaluator.h"

#define exprtk_disable_caseinsensitivity
#include <exprtk.hpp>

typedef exprtk::symbol_table<RZ::Real> exprtk_symtab_t;
typedef exprtk::expression<RZ::Real>   exprtk_expr_t;
typedef exprtk::parser<RZ::Real>       exprtk_parser_t;
typedef typename exprtk_parser_t::dependent_entity_collector::symbol_t exprtk_symbol_t;

////////////////////////////// ExprTkEvaluatorImpl /////////////////////////////
namespace RZ {
  struct RandU;
  struct RandN;

  struct ExprTkGenericCaller : public exprtk::ifunction<Real> {
    GenericCustomFunction *m_func = nullptr;

    ExprTkGenericCaller(GenericCustomFunction *func)
      : exprtk::ifunction<RZ::Real>(func->argc)
    {
      m_func = func;
    }

    Real
    operator()()
    {
      return m_func->evaluate(nullptr, 0);
    }

    Real
    operator()(
      Real const &a0)
    {
      Real args[] = {a0};
      return m_func->evaluate(args, sizeof(args) / sizeof(Real));
    }

    Real
    operator()(
      Real const &a0,
      Real const &a1)
    {
      Real args[] = {a0, a1};
      return m_func->evaluate(args, sizeof(args) / sizeof(Real));
    }

    Real
    operator()(
      Real const &a0,
      Real const &a1,
      Real const &a2)
    {
      Real args[] = {a0, a1, a2};
      return m_func->evaluate(args, sizeof(args) / sizeof(Real));
    }

    Real
    operator()(
      Real const &a0,
      Real const &a1,
      Real const &a2,
      Real const &a3)
    {
      Real args[] = {a0, a1, a2, a3};
      return m_func->evaluate(args, sizeof(args) / sizeof(Real));
    }

    Real
    operator()(
      Real const &a0,
      Real const &a1,
      Real const &a2,
      Real const &a3,
      Real const &a4)
    {
      Real args[] = {a0, a1, a2, a3, a4};
      return m_func->evaluate(args, sizeof(args) / sizeof(Real));
    }

    Real
    operator()(
      Real const &a0,
      Real const &a1,
      Real const &a2,
      Real const &a3,
      Real const &a4,
      Real const &a5)
    {
      Real args[] = {a0, a1, a2, a3, a4, a5};
      return m_func->evaluate(args, sizeof(args) / sizeof(Real));
    }

    Real
    operator()(
      Real const &a0,
      Real const &a1,
      Real const &a2,
      Real const &a3,
      Real const &a4,
      Real const &a5,
      Real const &a6)
    {
      Real args[] = {a0, a1, a2, a3, a4, a5, a6};
      return m_func->evaluate(args, sizeof(args) / sizeof(Real));
    }

    Real
    operator()(
      Real const &a0,
      Real const &a1,
      Real const &a2,
      Real const &a3,
      Real const &a4,
      Real const &a5,
      Real const &a6,
      Real const &a7)
    {
      Real args[] = {a0, a1, a2, a3, a4, a5, a6, a7};
      return m_func->evaluate(args, sizeof(args) / sizeof(Real));
    }
  };

  struct RandomFunction : public exprtk::ifunction<Real> {
    ExprRandomState *m_state    = nullptr;
    ExprRandomState *m_ownState = nullptr;
    uint64_t           m_epoch    = 0;
    Real               m_eval     = 0;

    RandomFunction(ExprRandomState *);
    ~RandomFunction();

    Real operator()(Real const &a, Real const &b);

    virtual Real evaluate(Real const &a, Real const &b) = 0;
  };

  struct RandU : public RandomFunction {
    using RandomFunction::RandomFunction;
    virtual Real evaluate(Real const &a, Real const &b) override;
  };

  struct RandN : public RandomFunction {
    using RandomFunction::RandomFunction;
    virtual Real evaluate(Real const &a, Real const &b) override;
  };

  class ExprTkEvaluatorImpl {
    exprtk_symtab_t        m_symTab;
    exprtk_expr_t          m_expr;
    exprtk_parser_t        m_parser;
    std::list<std::string> m_deps;
    std::list<ExprTkGenericCaller> m_customFuncs;
    ExprRandomState       *m_state;
    RandU                 *m_uniform;
    RandN                 *m_normal;

  public:
    ExprTkEvaluatorImpl(
      GenericEvaluatorSymbolDict *dict,
      ExprRandomState *state);
    virtual ~ExprTkEvaluatorImpl();

    bool compile(std::string const &name);
    Real evaluate();
    std::list<std::string> const &dependencies() const;
    std::string getLastParserError() const;
    bool registerCustomFunction(GenericCustomFunction *func);
  };
}

using namespace RZ;

RandomFunction::RandomFunction(ExprRandomState *state) 
  : exprtk::ifunction<RZ::Real>(2)
{
  if (state == nullptr) {
    m_ownState = new ExprRandomState();
    state = m_ownState;
  }

  m_state = state;
}

RandomFunction::~RandomFunction()
{
  if (m_ownState != nullptr)
    delete m_ownState;
}

Real
RandomFunction::operator()(Real const &a, Real const &b)
{
  if (m_state->epoch() != m_epoch) {
    m_eval = evaluate(a, b);
    m_epoch = m_state->epoch();
  }

  return m_eval;
}

Real
RandU::evaluate(Real const &a, Real const &b)
{
  return (b - a) * m_state->randu() + a;
}

Real
RandN::evaluate(Real const &a, Real const &b)
{
  return b * m_state->randn() + a;
}

ExprTkEvaluatorImpl::ExprTkEvaluatorImpl(
  GenericEvaluatorSymbolDict *dict,
  ExprRandomState *state)
{
  for (auto p : *dict)
    m_symTab.add_variable(p.first, p.second->value);

  m_uniform = new RandU(state);
  m_normal  = new RandN(state);

  m_symTab.add_constant("pi", M_PI);
  m_symTab.add_function("randu", *m_uniform);
  m_symTab.add_function("randn", *m_normal);
  
  m_expr.register_symbol_table(m_symTab);

  // To retrieve the symbol list
  m_parser.dec().collect_variables() = true;
}

ExprTkEvaluatorImpl::~ExprTkEvaluatorImpl()
{
  if (m_uniform != nullptr)
    delete m_uniform;

  if (m_normal != nullptr)
    delete m_normal;
}

bool
ExprTkEvaluatorImpl::registerCustomFunction(GenericCustomFunction *func)
{
  m_customFuncs.push_back(ExprTkGenericCaller(func));
  auto &last = m_customFuncs.back();
  return m_symTab.add_function(func->name, last);
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
ExprTkEvaluator::ExprTkEvaluator(
  GenericEvaluatorSymbolDict *dict,
  ExprRandomState *state) :
  GenericEvaluator(dict, state)
{
  p_impl = new ExprTkEvaluatorImpl(dict, randState());
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
ExprTkEvaluator::registerCustomFunction(GenericCustomFunction *func)
{
  GenericEvaluator::registerCustomFunction(func);
  return p_impl->registerCustomFunction(func);
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
