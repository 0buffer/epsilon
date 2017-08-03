#ifndef POINCARE_REEL_PART_H
#define POINCARE_REEL_PART_H

#include <poincare/function.h>

namespace Poincare {

class ReelPart : public Function {
public:
  ReelPart();
  Type type() const override;
  Expression * cloneWithDifferentOperands(Expression ** newOperands,
    int numberOfOperands, bool cloneOperands = true) const override;
private:
  Complex computeComplex(const Complex c, AngleUnit angleUnit) const override;
};

}

#endif


