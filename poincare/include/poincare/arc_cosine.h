#ifndef POINCARE_ARC_COSINE_H
#define POINCARE_ARC_COSINE_H

#include <poincare/function.h>

namespace Poincare {

class ArcCosine : public Function {
public:
  ArcCosine();
  Type type() const override;
  Expression * cloneWithDifferentOperands(Expression ** newOperands,
    int numberOfOperands, bool cloneOperands = true) const override;
private:
  Complex computeComplex(const Complex c, AngleUnit angleUnit) const override;
};

}

#endif
