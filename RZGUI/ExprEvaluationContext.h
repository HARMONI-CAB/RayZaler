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

#ifndef EXPREVALUATIONCONTEXT_H
#define EXPREVALUATIONCONTEXT_H

#include <ExprTkEvaluator.h>
#include <Recipe.h>
#include <map>

struct ExprEvaluationVar {
  std::string                     name;
  RZ::GenericEvaluatorSymbolDict *dict = nullptr;
  RZ::RecipeParameter             description;
  RZ::Real                        value = 0;

  ExprEvaluationVar &operator=(RZ::Real val);
};

class ExprEvaluationContext
{
  std::map<std::string, RZ::ExprTkEvaluator *> m_evaluators;
  std::map<std::string, ExprEvaluationVar>     m_varDescriptions;
  RZ::GenericEvaluatorSymbolDict               m_variables;
  std::string                                  m_lastCompileError;
  ExprEvaluationContext                       *m_parentCtx = nullptr;

  void addVariablesToEvaluator(RZ::ExprTkEvaluator *) const;

public:
  ExprEvaluationContext(ExprEvaluationContext *parent = nullptr);
  ~ExprEvaluationContext();

  bool defineExpression(std::string const &, std::string const &);

  void defineVariable(
      std::string const &,
      RZ::Real value = 0,
      RZ::Real min = -std::numeric_limits<RZ::Real>::infinity(),
      RZ::Real max = +std::numeric_limits<RZ::Real>::infinity());

  RZ::Real setVariable(std::string const &, RZ::Real);
  RZ::Real eval(std::string const &);
  std::list<std::string> expressions() const;
  std::string const getLastError() const;
  std::list<std::string> variables() const;
  ExprEvaluationVar &operator[] (std::string const &);
  RZ::Real operator() (std::string const &);
};

#endif // EXPREVALUATIONCONTEXT_H
