#ifndef POINCARE_BINARY_OPERATION_H
#define POINCARE_BINARY_OPERATION_H

#include <poincare/expression.h>
#include <poincare/matrix.h>
#include <poincare/complex.h>

namespace Poincare {

class BinaryOperation : public Expression {
public:
  BinaryOperation(Expression ** operands, bool cloneOperands = true);
  ~BinaryOperation();
  bool hasValidNumberOfArguments() const override;
  const Expression * operand(int i) const override;
  int numberOfOperands() const override;
  Expression * clone() const override;
protected:
  Expression * m_operands[2];
  Expression * privateEvaluate(Context& context, AngleUnit angleUnit) const override;
  virtual Expression * evaluateOnComplex(Complex * c, Complex * d, Context& context, AngleUnit angleUnit) const = 0;
  virtual Expression * evaluateOnMatrixAndComplex(Matrix * m, Complex * c, Context& context, AngleUnit angleUnit) const;
  virtual Expression * evaluateOnComplexAndMatrix(Complex * c, Matrix * m, Context& context, AngleUnit angleUnit) const;
  virtual Expression * evaluateOnMatrices(Matrix * m, Matrix * n, Context& context, AngleUnit angleUnit) const;
};

}

#endif
