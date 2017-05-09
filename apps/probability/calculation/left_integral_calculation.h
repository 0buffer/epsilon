#ifndef PROBABILITE_LEFT_INTEGRAL_CALCULATION_H
#define PROBABILITE_LEFT_INTEGRAL_CALCULATION_H

#include "calculation.h"

namespace Probability {

class LeftIntegralCalculation : public Calculation {
public:
  LeftIntegralCalculation();
  Type type() override;
  int numberOfParameters() override;
  I18n::Message legendForParameterAtIndex(int index) override;
  void setParameterAtIndex(float f, int index) override;
  float parameterAtIndex(int index) override;
  float upperBound() override;
private:
  void compute(int indexKnownElement) override;
  float m_upperBound;
  float m_result;
};

}

#endif
