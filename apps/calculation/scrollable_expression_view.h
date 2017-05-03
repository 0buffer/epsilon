#ifndef CALCULATION_SCROLLABLE_EXPRESSION_VIEW_H
#define CALCULATION_SCROLLABLE_EXPRESSION_VIEW_H

#include <escher.h>

namespace Calculation {

class ScrollableExpressionView : public ScrollableView, public ScrollViewDelegate {
public:
  ScrollableExpressionView(Responder * parentResponder);
  void setExpression(Poincare::ExpressionLayout * expressionLayout);
  void setBackgroundColor(KDColor backgroundColor);
  KDSize minimalSizeForOptimalDisplay() const override;
private:
  View * view() override;
  ExpressionView m_expressionView;
};

}

#endif
