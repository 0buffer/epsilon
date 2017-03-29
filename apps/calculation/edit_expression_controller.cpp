#include "edit_expression_controller.h"
#include "app.h"
#include "../apps_container.h"
#include <assert.h>

using namespace Shared;

namespace Calculation {

EditExpressionController::ContentView::ContentView(Responder * parentResponder, TableView * subview, TextFieldDelegate * textFieldDelegate) :
  View(),
  m_mainView(subview),
  m_textField(parentResponder, m_textBody, 255, textFieldDelegate)
{
  m_textBody[0] = 0;
}

int EditExpressionController::ContentView::numberOfSubviews() const {
  return 2;
}

View * EditExpressionController::ContentView::subviewAtIndex(int index) {
  View * views[2] = {m_mainView, &m_textField};
  return views[index];
}

void EditExpressionController::ContentView::layoutSubviews() {
  KDRect mainViewFrame(0, 0, bounds().width(), bounds().height() - k_textFieldHeight-k_separatorThickness);
  m_mainView->setFrame(mainViewFrame);
  KDRect inputViewFrame(0, bounds().height() - k_textFieldHeight, bounds().width(), k_textFieldHeight);
  m_textField.setFrame(inputViewFrame);
}

void  EditExpressionController::ContentView::drawRect(KDContext * ctx, KDRect rect) const {
  ctx->fillRect(KDRect(0, bounds().height() -k_textFieldHeight-k_separatorThickness, bounds().width(), k_separatorThickness), Palette::GreyMiddle);
}

TextField * EditExpressionController::ContentView::textField() {
  return &m_textField;
}

TableView * EditExpressionController::ContentView::mainView() {
  return m_mainView;
}

EditExpressionController::EditExpressionController(Responder * parentResponder, HistoryController * historyController, CalculationStore * calculationStore) :
  ViewController(parentResponder),
  m_contentView(this, (TableView *)historyController->view(), this),
  m_historyController(historyController),
  m_calculationStore(calculationStore)
{
}

View * EditExpressionController::view() {
  return &m_contentView;
}

const char * EditExpressionController::textBody() {
  return m_contentView.textField()->text();
}

void EditExpressionController::setTextBody(const char * text) {
  m_contentView.textField()->setEditing(true);
  m_contentView.textField()->setText(text);
}

bool EditExpressionController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Up) {
    if (m_calculationStore->numberOfCalculations() > 0) {
      m_contentView.textField()->setEditing(false);
      app()->setFirstResponder(m_historyController);
    }
    return true;
  }
  return false;
}

void EditExpressionController::didBecomeFirstResponder() {
  m_contentView.textField()->setEditing(true, false);
  app()->setFirstResponder(m_contentView.textField());
}

bool EditExpressionController::textFieldDidFinishEditing(::TextField * textField, const char * text) {
  App * calculationApp = (App *)app();
  m_calculationStore->push(textBody(), calculationApp->localContext());
  m_historyController->reload();
  m_contentView.mainView()->scrollToCell(0, m_historyController->numberOfRows()-1);
  m_contentView.textField()->setEditing(true);
  m_contentView.textField()->setText("");
  return true;
}

bool EditExpressionController::textFieldDidAbortEditing(::TextField * textField, const char * text) {
  m_contentView.textField()->setEditing(true);
  m_contentView.textField()->setText(text);
  return false;
}

TextFieldDelegateApp * EditExpressionController::textFieldDelegateApp() {
  return (App *)app();
}

}
