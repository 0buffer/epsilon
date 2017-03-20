#ifndef SHARED_GO_TO_PARAMETER_CONTROLLER_H
#define SHARED_GO_TO_PARAMETER_CONTROLLER_H

#include <escher.h>
#include "float_parameter_controller.h"
#include "curve_view_cursor.h"
#include "interactive_curve_view_range.h"
#include "function.h"

namespace Shared {

class GoToParameterController : public FloatParameterController {
public:
  GoToParameterController(Responder * parentResponder, InteractiveCurveViewRange * graphRange, CurveViewCursor * cursor, I18n::Message symbol);
  const char * title() override;
  void viewWillAppear() override;
  void setFunction(Function * function);
  int numberOfRows() override;
private:
  constexpr static float k_maxDisplayableFloat = 1E7f;
  void buttonAction() override;
  HighlightCell * reusableParameterCell(int index, int type) override;
  int reusableParameterCellCount(int type) override;
  float previousParameterAtIndex(int index) override;
  float parameterAtIndex(int index) override;
  bool setParameterAtIndex(int parameterIndex, float f) override;
  char m_draftTextBuffer[MessageTableCellWithEditableText::k_bufferLength];
  MessageTableCellWithEditableText m_abscisseCell;
  float m_previousParameter;
  InteractiveCurveViewRange * m_graphRange;
  CurveViewCursor * m_cursor;
  Function * m_function;
};

}

#endif
