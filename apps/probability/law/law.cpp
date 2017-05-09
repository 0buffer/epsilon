#include "law.h"
#include <math.h>
#include <float.h>

namespace Probability {

Law::Law() :
 Shared::CurveViewRange()
{
}
float Law::xGridUnit() {
  return computeGridUnit(Axis::X, xMin(), xMax());
}

float Law::cumulativeDistributiveFunctionAtAbscissa(float x) const {
  if (!isContinuous()) {
    int end = roundf(x);
    float result = 0.0f;
    for (int k = 0; k <=end; k++) {
      result += evaluateAtAbscissa(k);
      /* Avoid too long loop */
      if (result > k_maxProbability || k > k_maxNumberOfOperations) {
        break;
      }
    }
    return result;
  }
  return 0.0f;
}

float Law::rightIntegralFromAbscissa(float x) const {
  if (isContinuous()) {
    return 1.0f - cumulativeDistributiveFunctionAtAbscissa(x);
  }
  return 1.0f - cumulativeDistributiveFunctionAtAbscissa(x-1.0f);
}

float Law::finiteIntegralBetweenAbscissas(float a, float b) const {
  if (b < a) {
    return 0.0f;
  }
  if (isContinuous()) {
    return cumulativeDistributiveFunctionAtAbscissa(b) - cumulativeDistributiveFunctionAtAbscissa(a);
  }
  int start = roundf(a);
  int end = roundf(b);
  float result = 0.0f;
  for (int k = start; k <=end; k++) {
    result += evaluateAtAbscissa(k);
    /* Avoid too long loop */
    if (result > k_maxProbability || k-start > k_maxNumberOfOperations) {
      break;
    }
  }
  return result;
}

float Law::cumulativeDistributiveInverseForProbability(float * probability) {
  if (*probability >= 1.0f) {
    return INFINITY;
  }
  if (isContinuous()) {
    return 0.0f;
  }
  if (*probability <= 0.0f) {
    return 0.0f;
  }
  float p = 0.0f;
  int k = 0;
  while (p < *probability && k < k_maxNumberOfOperations) {
    p += evaluateAtAbscissa(k++);
  }
  if (k == k_maxNumberOfOperations) {
    *probability = 1.0f;
    return INFINITY;
  }
  *probability = p;
  if (isnan(*probability)) {
    return NAN;
  }
  return k-1;
}

float Law::rightIntegralInverseForProbability(float * probability) {
  if (isContinuous()) {
    float f = 1.0f - *probability;
    return cumulativeDistributiveInverseForProbability(&f);
  }
  if (*probability >= 1.0f) {
    return 0.0f;
  }
  if (*probability <= 0.0f) {
    return INFINITY;
  }
  float p = 0.0f;
  int k = 0;
  while (p < 1.0f - *probability && k < k_maxNumberOfOperations) {
    p += evaluateAtAbscissa(k++);
  }
  if (k == k_maxNumberOfOperations) {
    *probability = 1.0f;
    return INFINITY;
  }
  *probability = 1.0f - (p - evaluateAtAbscissa(k-1));
  if (isnan(*probability)) {
    return NAN;
  }
  return k-1;
}

}
