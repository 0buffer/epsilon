#include "symbols.h"
#include <string.h>
#include <kandinsky.h>
#include <ion.h>

void print(const char * message) {
#if QUIZ_USE_CONSOLE
  Ion::Console::writeLine(message);
#else
  static int line_y = 0;
  KDContext * ctx = KDIonContext::sharedContext();
  int line_height = KDText::stringSize("M").height();
  ctx->drawString(message, KDPoint(0, line_y));
  line_y += line_height;
  if (line_y > Ion::Display::Height) {
    line_y = 0;
    // Clear screen maybe?
  }
#endif
}

void ion_app() {
  int i = 0;
  while (quiz_cases[i] != NULL) {
    QuizCase c = quiz_cases[i];
    c();
    print(quiz_case_names[i]);
    i++;
  }
  print("ALL TESTS FINISHED");
#if !QUIZ_USE_CONSOLE
  while (1) {
    Ion::msleep(1000);
  }
#endif
}
