#include "toolbox_helpers.h"
#include <string.h>

namespace Shared {
namespace ToolboxHelpers {

int CursorIndexInCommand(const char * text) {
  for (int i = 0; i < strlen(text); i++) {
    if (text[i] == '(') {
      return i + 1;
    } else if (text[i] == '.') {
      return i;
    }
  }
  return strlen(text);
}

void TextToInsertForCommandMessage(I18n::Message message, char * buffer) {
  const char * messageText = I18n::translate(message);
  int currentNewTextIndex = 0;
  int numberOfOpenBrackets = 0;
  for (int i = 0; i < strlen(messageText); i++) {
    if (messageText[i] == '.') {
      currentNewTextIndex = 0;
      numberOfOpenBrackets = 0;
    } else if (messageText[i] == ')') {
      numberOfOpenBrackets--;
    }
    if (numberOfOpenBrackets == 0 || messageText[i] == ',')
    {
      buffer[currentNewTextIndex] = messageText[i];
      buffer[currentNewTextIndex + 1] = 0;
      currentNewTextIndex++;
    }
    if (messageText[i] == '(') {
      numberOfOpenBrackets++;
    }
  }
}

}
}
