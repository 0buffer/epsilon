#include <poincare/expression.h>
#include <poincare/preferences.h>
#include <poincare/symbol.h>
#include <poincare/dynamic_hierarchy.h>
#include <poincare/static_hierarchy.h>
#include <poincare/list_data.h>
#include <poincare/matrix_data.h>
#include <poincare/evaluation.h>
#include <poincare/undefined.h>
#include <poincare/simplification_root.h>
#include <cmath>
#include "expression_parser.hpp"
#include "expression_lexer.hpp"

int poincare_expression_yyparse(Poincare::Expression ** expressionOutput);

namespace Poincare {

static Expression::CircuitBreaker sCircuitBreaker = nullptr;

#include <stdio.h>

Expression * Expression::parse(char const * string) {
  if (string[0] == 0) {
    return nullptr;
  }
  YY_BUFFER_STATE buf = poincare_expression_yy_scan_string(string);
  Expression * expression = 0;
  if (poincare_expression_yyparse(&expression) != 0) {
    // Parsing failed because of invalid input or memory exhaustion
    if (expression != nullptr) {
      delete expression;
      expression = nullptr;
    }
  }
  poincare_expression_yy_delete_buffer(buf);

  return expression;
}

void Expression::setCircuitBreaker(CircuitBreaker cb) {
  sCircuitBreaker = cb;
}

bool Expression::shouldStopProcessing() {
  if (sCircuitBreaker == nullptr) {
    return false;
  }
  return sCircuitBreaker();
}

ExpressionLayout * Expression::createLayout(FloatDisplayMode floatDisplayMode, ComplexFormat complexFormat) const {
  switch (floatDisplayMode) {
    case FloatDisplayMode::Default:
      switch (complexFormat) {
        case ComplexFormat::Default:
          return privateCreateLayout(Preferences::sharedPreferences()->displayMode(), Preferences::sharedPreferences()->complexFormat());
        default:
          return privateCreateLayout(Preferences::sharedPreferences()->displayMode(), complexFormat);
      }
    default:
      switch (complexFormat) {
        case ComplexFormat::Default:
          return privateCreateLayout(floatDisplayMode, Preferences::sharedPreferences()->complexFormat());
        default:
          return privateCreateLayout(floatDisplayMode, complexFormat);
      }
  }
}

void Expression::simplifyAndBeautify(Expression ** expressionAddress) {
  SimplificationRoot root(*expressionAddress);
  root.simplify();
  root.beautify();
  *expressionAddress = (Expression *)(root.operand(0));
}

Expression * Expression::simplify() {
  for (int i = 0; i < numberOfOperands(); i++) {
    ((Expression *)operand(i))->simplify();
  }
  return immediateSimplify();
}

Expression * Expression::beautify() {
  Expression * e = immediateBeautify();
  for (int i = 0; i < e->numberOfOperands(); i++) {
    ((Expression *)e->operand(i))->beautify();
  }
}

Expression * Expression::immediateSimplify() {
  for (int i = 0; i< numberOfOperands(); i++) {
    if (operand(i)->type() == Type::Undefined) {
      return replaceWith(new Undefined(), true);
    }
  }
  return this;
}

bool Expression::hasAncestor(const Expression * e) const {
  assert(m_parent != this);
  if (m_parent == e) {
    return true;
  }
  if (m_parent == nullptr) {
    return false;
  }
  return m_parent->hasAncestor(e);
}

Expression * Expression::replaceWith(Expression * newOperand, bool deleteAfterReplace) {
  assert(m_parent != nullptr);
  m_parent->replaceOperand(this, newOperand, deleteAfterReplace);
  return newOperand;
}

/*void Expression::removeFromParent() {
  assert(m_parent != nullptr);
  assert(m_parent->type() == Expression::Type::Addition || m_parent->type() == Expression::Type::Multiplication); // Weak assertion. We just want to make sure this is actually a DynamicHierarchy
  static_cast<DynamicHierarchy *>(m_parent)->removeOperand(this);
}*/

int Expression::compareTo(const Expression * e) const {
  if (this->type() > e->type()) {
    return -(e->compareTo(this));
  } else if (this->type() == e->type()) {
    return compareToSameTypeExpression(e);
  } else {
    return compareToGreaterTypeExpression(e);
  }
}

template<typename T> Evaluation<T> * Expression::evaluate(Context& context, AngleUnit angleUnit) const {
  switch (angleUnit) {
    case AngleUnit::Default:
      return privateEvaluate(T(), context, Preferences::sharedPreferences()->angleUnit());
    default:
      return privateEvaluate(T(), context, angleUnit);
  }
}

template<typename T> T Expression::approximate(Context& context, AngleUnit angleUnit) const {
  Evaluation<T> * evaluation = evaluate<T>(context, angleUnit);
  T result = evaluation->toScalar();
  delete evaluation;
  return result;
}

template<typename T> T Expression::approximate(const char * text, Context& context, AngleUnit angleUnit) {
  Expression * exp = parse(text);
  if (exp == nullptr) {
    return NAN;
  }
  Evaluation<T> * evaluation = exp->evaluate<T>(context, angleUnit);
  delete exp;
  T result = evaluation->toScalar();
  delete evaluation;
  return result;
}

template<typename T> T Expression::epsilon() {
  static T epsilon = sizeof(T) == sizeof(double) ? 1E-15 : 1E-7f;
  return epsilon;
}

}

template Poincare::Evaluation<double> * Poincare::Expression::evaluate<double>(Context& context, AngleUnit angleUnit) const;
template Poincare::Evaluation<float> * Poincare::Expression::evaluate<float>(Context& context, AngleUnit angleUnit) const;
template double Poincare::Expression::approximate<double>(char const*, Poincare::Context&, Poincare::Expression::AngleUnit);
template float Poincare::Expression::approximate<float>(char const*, Poincare::Context&, Poincare::Expression::AngleUnit);
template double Poincare::Expression::approximate<double>(Poincare::Context&, Poincare::Expression::AngleUnit) const;
template float Poincare::Expression::approximate<float>(Poincare::Context&, Poincare::Expression::AngleUnit) const;
template double Poincare::Expression::epsilon<double>();
template float Poincare::Expression::epsilon<float>();
