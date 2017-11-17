#include "console_controller.h"
#include "script.h"
#include "helpers.h"
#include <apps/i18n.h>
#include <assert.h>

extern "C" {
#include <stdlib.h>
}

namespace Code {

ConsoleController::ConsoleController(Responder * parentResponder, ScriptStore * scriptStore) :
  ViewController(parentResponder),
  SelectableTableViewDataSource(),
  TextFieldDelegate(),
  m_rowHeight(KDText::charSize(k_fontSize).height()),
  m_selectableTableView(this, this, 0, 1, 0, Metric::CommonRightMargin, 3, Metric::HistoryHorizontalMargin, this, this, true, true, KDColorWhite),
  m_editCell(this, this),
  m_pythonHeap(nullptr),
  m_scriptStore(scriptStore)
{
}

ConsoleController::~ConsoleController() {
  unloadPythonEnvironment();
}

bool ConsoleController::loadPythonEnvironment() {
  if(pythonEnvironmentIsLoaded()) {
    return true;
  }
  emptyOutputAccumulationBuffer();
  m_pythonHeap = (char *)malloc(k_pythonHeapSize);
  if (m_pythonHeap == nullptr) {
    // In DEBUG mode, the assert at the end of malloc would have already failed
    // and the program crashed.
    return false;
  }
  MicroPython::init(m_pythonHeap, m_pythonHeap + k_pythonHeapSize);
  MicroPython::registerScriptProvider(m_scriptStore);
  autoImport();
  return true;
}

void ConsoleController::unloadPythonEnvironment() {
  if (pythonEnvironmentIsLoaded()) {
    m_consoleStore.clear();
    MicroPython::deinit();
    free(m_pythonHeap);
    m_pythonHeap = nullptr;
  }
}

bool ConsoleController::pythonEnvironmentIsLoaded() {
  return (m_pythonHeap != nullptr);
}

void ConsoleController::autoImport() {
  for (int i = 0; i < m_scriptStore->numberOfScripts(); i++) {
    autoImportScriptAtIndex(i);
  }
}

void ConsoleController::runAndPrintForCommand(const char * command) {
  m_consoleStore.pushCommand(command, strlen(command));
  assert(m_outputAccumulationBuffer[0] == '\0');
  runCode(command);
  flushOutputAccumulationBufferToStore();
  m_consoleStore.deleteLastLineIfEmpty();
  m_selectableTableView.reloadData();
  m_editCell.setEditing(true);
}

void ConsoleController::removeExtensionIfAny(char * name) {
  int nameLength = strlen(name);
  if (nameLength<4) {
    return;
  }
  if (strcmp(&name[nameLength-3], ScriptStore::k_scriptExtension) == 0) {
    name[nameLength-3] = 0;
  }
}

void ConsoleController::viewWillAppear() {
  assert(pythonEnvironmentIsLoaded());
  m_selectableTableView.reloadData();
  m_selectableTableView.selectCellAtLocation(0, m_consoleStore.numberOfLines());
  m_editCell.setEditing(true);
}

void ConsoleController::didBecomeFirstResponder() {
  app()->setFirstResponder(&m_editCell);
}

bool ConsoleController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Up) {
    if (m_consoleStore.numberOfLines() > 0 && m_selectableTableView.selectedRow() > 0) {
      m_editCell.setEditing(false);
      m_editCell.setText("");
      m_selectableTableView.selectCellAtLocation(0, m_consoleStore.numberOfLines()-1);
      app()->setFirstResponder(&m_selectableTableView);
      return true;
    }
  } else if (event == Ion::Events::OK || event == Ion::Events::EXE) {
    if (m_consoleStore.numberOfLines() > 0) {
      const char * text = m_consoleStore.lineAtIndex(m_selectableTableView.selectedRow()).text();
      m_editCell.setEditing(true);
      m_selectableTableView.selectCellAtLocation(0, m_consoleStore.numberOfLines());
      app()->setFirstResponder(&m_editCell);
      m_editCell.setText(text);
      return true;
    }
  } else if (event == Ion::Events::Copy) {
    int row = m_selectableTableView.selectedRow();
    if (row < m_consoleStore.numberOfLines()) {
      Clipboard::sharedClipboard()->store(m_consoleStore.lineAtIndex(row).text());
      return true;
    }
  }
  return false;
}

int ConsoleController::numberOfRows() {
  return m_consoleStore.numberOfLines()+1;
}

KDCoordinate ConsoleController::rowHeight(int j) {
  return m_rowHeight;
}

KDCoordinate ConsoleController::cumulatedHeightFromIndex(int j) {
  return j*rowHeight(0);
}

int ConsoleController::indexFromCumulatedHeight(KDCoordinate offsetY ){
  return offsetY/rowHeight(0);
}

HighlightCell * ConsoleController::reusableCell(int index, int type) {
  assert(index >= 0);
  if (type == LineCellType) {
    assert(index < k_numberOfLineCells);
    return m_cells+index;
  } else {
    assert(type == EditCellType);
    assert(index == 0);
    return &m_editCell;
  }
}

int ConsoleController::reusableCellCount(int type) {
  if (type == LineCellType) {
    return k_numberOfLineCells;
  } else {
    return 1;
  }
}

int ConsoleController::typeAtLocation(int i, int j) {
  assert(i == 0);
  assert(j >= 0);
  if (j < m_consoleStore.numberOfLines()) {
    return LineCellType;
  } else {
    assert(j == m_consoleStore.numberOfLines());
    return EditCellType;
  }
}

void ConsoleController::willDisplayCellAtLocation(HighlightCell * cell, int i, int j) {
  assert(i == 0);
  if (j < m_consoleStore.numberOfLines()) {
    static_cast<ConsoleLineCell *>(cell)->setLine(m_consoleStore.lineAtIndex(j));
  }
}

void ConsoleController::tableViewDidChangeSelection(SelectableTableView * t, int previousSelectedCellX, int previousSelectedCellY) {
  if (t->selectedRow() == m_consoleStore.numberOfLines()) {
    m_editCell.setEditing(true);
    app()->setFirstResponder(&m_editCell);
  }
}

bool ConsoleController::textFieldShouldFinishEditing(TextField * textField, Ion::Events::Event event) {
  return event == Ion::Events::OK || event == Ion::Events::EXE;
}

bool ConsoleController::textFieldDidReceiveEvent(TextField * textField, Ion::Events::Event event) {
  const char * pythonText = Helpers::PythonTextForEvent(event);
  if (pythonText == nullptr) {
    return false;
  }
  textField->setEditing(true, false);
  if (textField->insertTextAtLocation(pythonText, textField->cursorLocation())) {
    textField->setCursorLocation(textField->cursorLocation()+strlen(pythonText));
    if (pythonText[strlen(pythonText)-1] == ')') {
      textField->setCursorLocation(textField->cursorLocation()-1);
    }
  }
  return true;
}

bool ConsoleController::textFieldDidFinishEditing(TextField * textField, const char * text, Ion::Events::Event event) {
  runAndPrintForCommand(text);
  textField->setText("");
  m_selectableTableView.selectCellAtLocation(0, m_consoleStore.numberOfLines());
  return true;
}

bool ConsoleController::textFieldDidAbortEditing(TextField * textField, const char * text) {
  stackViewController()->pop();
  return true;
}

Toolbox * ConsoleController::toolboxForTextField(TextField * textFied) {
  return nullptr;
}

/* printText is called by the Python machine.
 * The text argument is not always null-terminated. */
void ConsoleController::printText(const char * text, size_t length) {
  int textCutIndex = firstNewLineCharIndex(text, length);
  // If there is no new line in text, just append it to the output accumulation
  // buffer.
  if (textCutIndex >= length) {
    appendTextToOutputAccumulationBuffer(text, length);
    return;
  }
  // If there is a new line in the middle of the text, we have to store at least
  // two new console lines in the console store.
  if (textCutIndex < length - 1) {
    printText(text, textCutIndex + 1);
    printText(&text[textCutIndex+1], length - (textCutIndex + 1));
    return;
  }
  // If there is a new line at the end of the text, we have to store the line in
  // the console store.
  if (textCutIndex == length - 1) {
    appendTextToOutputAccumulationBuffer(text, length-1);
    flushOutputAccumulationBufferToStore();
  }
}

void ConsoleController::autoImportScriptAtIndex(int index) {
  const char * importCommand1 = "from ";
  const char * importCommand2 = " import *";
  int lenImportCommand1 = strlen(importCommand1);
  int lenImportCommand2 = strlen(importCommand2);
  Script script = m_scriptStore->scriptAtIndex(index);
  if (script.autoImport()) {
    // Remove the name extension ".py" if there is one.
    int scriptOriginalNameLength = strlen(script.name());
    char scriptNewName[scriptOriginalNameLength];
    memcpy(scriptNewName, script.name(), scriptOriginalNameLength + 1);
    removeExtensionIfAny(scriptNewName);
    int scriptNewNameLength = strlen(scriptNewName);
    // Create the command "from scriptName import *".
    char command[lenImportCommand1 + scriptNewNameLength + lenImportCommand2 + 1];
    memcpy(command, importCommand1, lenImportCommand1);
    memcpy(&command[lenImportCommand1], scriptNewName, scriptNewNameLength);
    memcpy(&command[lenImportCommand1 + scriptNewNameLength], importCommand2, lenImportCommand2 + 1);
    runAndPrintForCommand(command);
  }
}

void ConsoleController::flushOutputAccumulationBufferToStore() {
  m_consoleStore.pushResult(m_outputAccumulationBuffer, strlen(m_outputAccumulationBuffer));
  emptyOutputAccumulationBuffer();
}

void ConsoleController::appendTextToOutputAccumulationBuffer(const char * text, size_t length) {
  int endOfAccumulatedText = strlen(m_outputAccumulationBuffer);
  int spaceLeft = k_outputAccumulationBufferSize - endOfAccumulatedText;
  if (spaceLeft > length) {
    memcpy(&m_outputAccumulationBuffer[endOfAccumulatedText], text, length);
    return;
  }
  memcpy(&m_outputAccumulationBuffer[endOfAccumulatedText], text, spaceLeft-1);
  flushOutputAccumulationBufferToStore();
  appendTextToOutputAccumulationBuffer(&text[spaceLeft-1], length - (spaceLeft - 1));
}

void ConsoleController::emptyOutputAccumulationBuffer() {
  for (int i = 0; i < k_outputAccumulationBufferSize; i++) {
    m_outputAccumulationBuffer[i] = 0;
  }
}

int ConsoleController::firstNewLineCharIndex(const char * text, size_t length) {
  int index = 0;
  while (index < length) {
    if (text[index] == '\n') {
      return index;
    }
    index++;
  }
  return index;
}

StackViewController * ConsoleController::stackViewController() {
 return static_cast<StackViewController *>(parentResponder());
}

}
