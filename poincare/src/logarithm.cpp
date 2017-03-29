#include <poincare/logarithm.h>
extern "C" {
#include <assert.h>
#include <math.h>
#include <stdlib.h>
}
#include "layout/baseline_relative_layout.h"
#include "layout/horizontal_layout.h"
#include "layout/parenthesis_layout.h"
#include "layout/string_layout.h"

namespace Poincare {

Logarithm::Logarithm() :
  Function("log", 2)
{
}

bool Logarithm::hasValidNumberOfArguments() const {
  if (m_numberOfArguments != 1 && m_numberOfArguments != 2) {
    return false;
  }
  for (int i = 0; i < m_numberOfArguments; i++) {
    if (!m_args[i]->hasValidNumberOfArguments()) {
      return false;
    }
  }
  return true;
}

Expression::Type Logarithm::type() const {
  return Type::Logarithm;
}

Expression * Logarithm::cloneWithDifferentOperands(Expression** newOperands,
        int numberOfOperands, bool cloneOperands) const {
  assert(newOperands != nullptr);
  Logarithm * l = new Logarithm();
  l->setArgument(newOperands, numberOfOperands, cloneOperands);
  return l;
}

float Logarithm::privateApproximate(Context& context, AngleUnit angleUnit) const {
  assert(angleUnit != AngleUnit::Default);
  if (m_numberOfArguments == 1) {
    return log10f(m_args[0]->approximate(context, angleUnit));
  }
  return log10f(m_args[1]->approximate(context, angleUnit))/log10f(m_args[0]->approximate(context, angleUnit));
}

ExpressionLayout * Logarithm::privateCreateLayout(FloatDisplayMode floatDisplayMode, ComplexFormat complexFormat) const {
  assert(floatDisplayMode != FloatDisplayMode::Default);
  assert(complexFormat != ComplexFormat::Default);
  if (m_numberOfArguments == 1) {
    return Function::privateCreateLayout(floatDisplayMode, complexFormat);
  }
  ExpressionLayout * childrenLayouts[2];
  childrenLayouts[0] = new BaselineRelativeLayout(new StringLayout(m_name, strlen(m_name)), m_args[0]->createLayout(floatDisplayMode, complexFormat), BaselineRelativeLayout::Type::Subscript);
  childrenLayouts[1] = new ParenthesisLayout(m_args[1]->createLayout(floatDisplayMode, complexFormat));
  return new HorizontalLayout(childrenLayouts, 2);
}

}
