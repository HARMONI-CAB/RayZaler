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

#include "ExprEvaluationContext.h"
#include <stdexcept>
#include <Logger.h>

ExprEvaluationVar &
ExprEvaluationVar::operator=(RZ::Real val)
{
  this->value = val;

  auto &dict = *this->dict;
  auto var = dict[name];

  if (var->test(val))
    dict[name]->value = val;
  else
    RZError("Cannot set %s = %g: value out of bounds\n", name.c_str(), val);

  return *this;
}

ExprEvaluationContext::ExprEvaluationContext(ExprEvaluationContext *ctx)
{
  m_parentCtx = ctx;
}

ExprEvaluationContext::~ExprEvaluationContext()
{
  for (auto &p : m_evaluators)
    delete p.second;
}

bool
ExprEvaluationContext::defineExpression(
    std::string const &name,
    std::string const &expr)
{
  RZ::ExprTkEvaluator *evaluator = new RZ::ExprTkEvaluator(&m_variables);

  if (m_parentCtx != nullptr)
    m_parentCtx->addVariablesToEvaluator(evaluator);

  if (!evaluator->compile(expr)) {
    m_lastCompileError = evaluator->getLastParserError();
    delete evaluator;
    evaluator = nullptr;
  }

  if (evaluator != nullptr) {
    auto it = m_evaluators.find(name);
    if (it != m_evaluators.end())
      delete it->second;

    m_evaluators[name] = evaluator;
  }

  return evaluator != nullptr;
}

std::string const
ExprEvaluationContext::getLastError() const
{
  return m_lastCompileError;
}

std::list<std::string>
ExprEvaluationContext::variables() const
{
  std::list<std::string> vars;

  for (auto &p : m_varDescriptions)
    vars.push_back(p.first);

  return vars;
}

std::list<std::string>
ExprEvaluationContext::expressions() const
{
  std::list<std::string> vars;

  for (auto &p : m_evaluators)
    vars.push_back(p.first);

  return vars;
}

void
ExprEvaluationContext::addVariablesToEvaluator(
    RZ::ExprTkEvaluator *evaluator) const
{
  evaluator->addVariables(&m_variables);

  if (m_parentCtx != nullptr)
    m_parentCtx->addVariablesToEvaluator(evaluator);
}

void
ExprEvaluationContext::defineVariable(
    std::string const &name,
    RZ::Real value,
    RZ::Real min,
    RZ::Real max)
{
  if (m_varDescriptions.find(name) == m_varDescriptions.end()) {
    m_varDescriptions[name].name = name;
    m_varDescriptions[name].dict = &m_variables;
  }

  m_varDescriptions[name].description.min        = min;
  m_varDescriptions[name].description.max        = max;
  m_varDescriptions[name].description.defaultVal = value;


  if (m_variables.find(name) == m_variables.end())
    m_variables[name] = new RZ::GenericModelParam();

  m_variables[name]->description    = &m_varDescriptions[name].description;
  m_variables[name]->value          = value;
}

RZ::Real
ExprEvaluationContext::setVariable(std::string const &name, RZ::Real val)
{
  auto it = m_varDescriptions.find(name);

  if (it == m_varDescriptions.end())
    defineVariable(name, val);
  else
    it->second = val;

  return val;
}

RZ::Real
ExprEvaluationContext::eval(std::string const &name)
{
  auto it = m_evaluators.find(name);

  if (it == m_evaluators.end())
    throw std::runtime_error("No such expression `" + name + "'");

  return it->second->evaluate();
}

ExprEvaluationVar &
ExprEvaluationContext::operator[] (std::string const &name)
{
  auto it = m_varDescriptions.find(name);

  if (it == m_varDescriptions.end())
    defineVariable(name);

  return m_varDescriptions[name];
}

RZ::Real
ExprEvaluationContext::operator() (std::string const &name)
{
  return eval(name);
}
