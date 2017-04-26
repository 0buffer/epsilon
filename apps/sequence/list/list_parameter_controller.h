#ifndef SEQUENCE_LIST_PARAM_CONTROLLER_H
#define SEQUENCE_LIST_PARAM_CONTROLLER_H

#include "../../shared/list_parameter_controller.h"
#include "../sequence.h"
#include "../sequence_store.h"
#include "type_parameter_controller.h"

namespace Sequence {

class ListController;

class ListParameterController : public Shared::ListParameterController {
public:
  ListParameterController(ListController * list, SequenceStore * sequenceStore);
  const char * title() override;
  bool handleEvent(Ion::Events::Event event) override;
  void setFunction(Shared::Function * function) override;
  int numberOfRows() override;
  HighlightCell * reusableCell(int index) override;
  int reusableCellCount() override;
  void willDisplayCellForIndex(HighlightCell * cell, int index) override;
private:
  /* TODO: implement function color choice */
  constexpr static int k_totalNumberOfCell = 3;//4
  MessageTableCellWithChevronAndExpression m_typeCell;
  TypeParameterController m_typeParameterController;
  Sequence * m_sequence;
};

}

#endif
