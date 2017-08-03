#ifndef POINCARE_NAPERIAN_LOGARITHM_H
#define POINCARE_NAPERIAN_LOGARITHM_H

#include <poincare/function.h>

namespace Poincare {

class NaperianLogarithm : public Function {
public:
  NaperianLogarithm();
  Type type() const override;
  Expression * cloneWithDifferentOperands(Expression ** newOperands,
      int numberOfOperands, bool cloneOperands = true) const override;
private:
  Complex computeComplex(const Complex c, AngleUnit angleUnit) const override;
};

}

#endif
