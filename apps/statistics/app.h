#ifndef STAT_APP_H
#define STAT_APP_H

#include <escher.h>
#include "box_controller.h"
#include "calculation_controller.h"
#include "store.h"
#include "store_controller.h"
#include "histogram_controller.h"
#include "../shared/text_field_delegate_app.h"

namespace Statistics {

class App : public Shared::TextFieldDelegateApp {
public:
  class Descriptor : public ::App::Descriptor {
  public:
    App * build(Container * container) override;
    I18n::Message name() override;
    I18n::Message upperName() override;
    const Image * icon() override;
  };
  static Descriptor * buildDescriptor();
private:
  App(Container * container, Descriptor * descriptor);
  Store m_store;
  CalculationController m_calculationController;
  AlternateEmptyViewController m_calculationAlternateEmptyViewController;
  ButtonRowController m_calculationHeader;
  BoxController m_boxController;
  AlternateEmptyViewController m_boxAlternateEmptyViewController;
  ButtonRowController m_boxHeader;
  HistogramController m_histogramController;
  AlternateEmptyViewController m_histogramAlternateEmptyViewController;
  ButtonRowController m_histogramHeader;
  StackViewController m_histogramStackViewController;
  StoreController m_storeController;
  ButtonRowController m_storeHeader;
  StackViewController m_storeStackViewController;
  TabViewController m_tabViewController;
};

}

#endif
