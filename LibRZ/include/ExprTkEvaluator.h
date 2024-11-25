//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

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
