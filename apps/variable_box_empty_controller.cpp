#include "variable_box_empty_controller.h"
#include <poincare/layout_helper.h>
#include "graph/storage_cartesian_function_store.h"
#include <apps/i18n.h>
#include <assert.h>

using namespace Poincare;
using namespace Ion;

VariableBoxEmptyController::VariableBoxEmptyView::VariableBoxEmptyView() :
  m_layoutExample(0.5f, 0.5f, KDColorBlack, Palette::WallScreen)
{
  for (int i = 0; i < k_numberOfMessages; i++) {
    m_messages[i].setFont(k_font);
    m_messages[i].setAlignment(0.5f, 0.5f);
    m_messages[i].setBackgroundColor(Palette::WallScreen);
  }
  m_messages[0].setAlignment(0.5f,1.0f);
  m_messages[k_numberOfMessages-1].setAlignment(0.5f,0.0f);
}

void VariableBoxEmptyController::VariableBoxEmptyView::setMessages(I18n::Message *  message) {
  for (int i = 0; i < k_numberOfMessages; i++) {
    m_messages[i].setMessage(message[i]);
  }
}

void VariableBoxEmptyController::VariableBoxEmptyView::setLayout(Poincare::Layout layout) {
  m_layoutExample.setLayout(layout);
}

int VariableBoxEmptyController::VariableBoxEmptyView::numberOfSubviews() const {
  return k_numberOfMessages+1;
}

View * VariableBoxEmptyController::VariableBoxEmptyView::subviewAtIndex(int index) {
  if (index == k_layoutRowIndex) {
    return &m_layoutExample;
  }
  if (index < k_layoutRowIndex) {
    return &m_messages[index];
  }
  return &m_messages[index-1];
}

void VariableBoxEmptyController::VariableBoxEmptyView::layoutSubviews() {
  KDCoordinate width = bounds().width();
  KDCoordinate height = bounds().height();
  KDCoordinate textHeight = k_font->glyphSize().height();
  KDCoordinate layoutHeight = m_layoutExample.minimalSizeForOptimalDisplay().height();
  KDCoordinate margin = (height - k_numberOfMessages*textHeight-layoutHeight)/2;
  m_layoutExample.setFrame(KDRect(0, margin+k_layoutRowIndex*textHeight, width, layoutHeight));
  KDCoordinate currentHeight = 0;
  for (uint8_t i = 0; i < k_numberOfMessages; i++) {
    if (i == k_layoutRowIndex) {
      currentHeight += layoutHeight;
    }
    KDCoordinate h = i == 0 || i == k_numberOfMessages - 1 ? textHeight+margin : textHeight;
    m_messages[i].setFrame(KDRect(0, currentHeight, width, h));
    currentHeight += h;
  }
}

VariableBoxEmptyController::VariableBoxEmptyController() :
  ViewController(nullptr),
  m_view()
{
}

View * VariableBoxEmptyController::view() {
  return &m_view;
}

void VariableBoxEmptyController::viewDidDisappear() {
  m_view.setLayout(Layout());
}

void VariableBoxEmptyController::setType(Type type) {
  I18n::Message messages[VariableBoxEmptyView::k_numberOfMessages] = {I18n::Message::Default, I18n::Message::Default, I18n::Message::Default, I18n::Message::EnableCharacters};
  Layout layout;
  switch (type) {
    case Type::Expressions:
    {
      messages[0] = I18n::Message::EmptyExpressionBox0;
      messages[1] = I18n::Message::EmptyExpressionBox1;
      messages[2] = I18n::Message::EmptyExpressionBox2;
      char storeExpression[] = {'3', Ion::Charset::Sto, 'A'};
      layout = LayoutHelper::String(storeExpression, sizeof(storeExpression), VariableBoxEmptyView::k_font);
      break;
    }
    case Type::Functions:
    {
      messages[0] = I18n::Message::EmptyFunctionBox0;
      messages[1] = I18n::Message::EmptyFunctionBox1;
      messages[2] = I18n::Message::EmptyFunctionBox2;
      char storeFunction[] = {'3', '+', Graph::StorageCartesianFunctionStore::Symbol(), Ion::Charset::Sto, 'f', '(', Graph::StorageCartesianFunctionStore::Symbol(), ')'};
      layout = LayoutHelper::String(storeFunction, sizeof(storeFunction), VariableBoxEmptyView::k_font);
      break;
    }
    default:
      assert(false);
  }
  m_view.setMessages(messages);
  m_view.setLayout(layout);
}
