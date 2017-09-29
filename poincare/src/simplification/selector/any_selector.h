#ifndef POINCARE_SIMPLIFICATION_SELECTOR_ANY_SELECTOR_H
#define POINCARE_SIMPLIFICATION_SELECTOR_ANY_SELECTOR_H

#include "selector.h"

namespace Poincare {
namespace Simplification {

class AnySelector : public Selector {
public:
  using Selector::Selector;
  bool acceptsLocationInCombination(const Combination * combination, int location) const override {
    return true;
  }
  bool immediateMatch(const Expression * e) const override {
    return true;
  }
};

}
}

#endif
