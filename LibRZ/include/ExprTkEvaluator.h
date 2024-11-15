#ifndef _EXPRTK_EVALUATOR_H
#define _EXPRTK_EVALUATOR_H

#include "GenericCompositeModel.h"
#include <random>

namespace RZ {
  class ExprTkEvaluatorImpl;

  class ExprTkEvaluator : public GenericEvaluator {
      ExprTkEvaluatorImpl *p_impl = nullptr;
      
    public:
      ExprTkEvaluator(
        const GenericEvaluatorSymbolDict *,
        ExprRandomState *state = nullptr);
      virtual ~ExprTkEvaluator();

      virtual std::list<std::string> dependencies() const override;
      virtual bool compile(std::string const &) override;
      virtual Real evaluate() override;
      std::string getLastParserError() const;
      virtual bool registerCustomFunction(GenericCustomFunction *) override;

      void addVariables(const GenericEvaluatorSymbolDict *);
  };
}

#endif // _EXPRTK_EVALUATOR_H
