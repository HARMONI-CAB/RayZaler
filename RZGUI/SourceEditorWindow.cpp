#include "SourceEditorWindow.h"
#include "ui_SourceEditorWindow.h"
#include "RZMHighLighter.h"
#include <QLabel>
#include <QMessageBox>
#include <QTextCharFormat>
#include "GUIHelpers.h"

SourceEditorParserContext::SourceEditorParserContext(
    RZ::Recipe *recipe,
    QTextEdit *edit) : RZ::ParserContext(recipe)
{
  m_source = edit->toPlainText().toStdString();
}

int
SourceEditorParserContext::read()
{
  if (m_pos >= m_source.size())
    return -1;

  return m_source[m_pos++];
}

SourceEditorWindow::SourceEditorWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::SourceEditorWindow)
{
  QFont font;
  QTextCharFormat fmt;

  font.setFamily("Cascadia Mono PL");
  font.setFixedPitch(true);
  font.setPointSize(10);

  ui->setupUi(this);

  m_lineLabel = new QLabel();
  m_colLabel = new QLabel();

  m_lineLabel->setMinimumWidth(64);
  m_colLabel->setMinimumWidth(64);

  ui->statusbar->addPermanentWidget(m_lineLabel);
  ui->statusbar->addPermanentWidget(m_colLabel);

  ui->sourceTextEdit->setFont(font);
  ui->sourceTextEdit->setUndoRedoEnabled(true);

  m_highlighter = new RZMHighLighter(ui->sourceTextEdit->document());
  //fmt.setBackground(ui->sourceTextEdit->textBackgroundColor());
  //m_highlighter->defineFormat("background", fmt);

  connectAll();
}

void
SourceEditorWindow::connectAll()
{
  connect(
        ui->sourceTextEdit,
        SIGNAL(cursorPositionChanged()),
        this,
        SLOT(onCursorChanged()));

  connect(
        ui->sourceTextEdit,
        SIGNAL(undoAvailable(bool)),
        this,
        SLOT(onUndoAvailable(bool)));

  connect(
        ui->sourceTextEdit,
        SIGNAL(redoAvailable(bool)),
        this,
        SLOT(onRedoAvailable(bool)));


  connect(
        ui->actionBuildModel,
        SIGNAL(triggered(bool)),
        this,
        SIGNAL(build()));

  connect(
        ui->sourceTextEdit,
        SIGNAL(textChanged()),
        this,
        SLOT(onTextEditChanged()));

  connect(
        ui->actionUndo,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onUndo()));

  connect(
        ui->actionRedo,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onRedo()));


}

void
SourceEditorWindow::refreshUi()
{
  if (m_changed) {
    setWindowTitle("Source editor - " + QString::fromStdString(m_fileName) + " [changed]");
  } else {
    setWindowTitle("Source editor - " + QString::fromStdString(m_fileName));
  }

  ui->actionUndo->setEnabled(ui->sourceTextEdit->document()->isUndoAvailable());
  ui->actionRedo->setEnabled(ui->sourceTextEdit->document()->isRedoAvailable());
}

void
SourceEditorWindow::setFileName(std::string const &fileName)
{
  m_fileName = fileName;
  refreshUi();
}

void
SourceEditorWindow::loadFromFp(FILE *fp)
{
  off_t curr = ftell(fp);
  std::vector<char> buffer;
  QString text;
  int size;

  buffer.resize(4096 + 1);

  fseek(fp, 0, SEEK_SET);
  m_original = "";

  while (!feof(fp)) {
    size = fread(buffer.data(), sizeof(char), buffer.size() - 1, fp);
    if (size > 0) {
      buffer.data()[size] = 0;
      text += buffer.data();
      m_original += buffer.data();
    }
  }

  fseek(fp, curr, SEEK_SET);

  ui->sourceTextEdit->setPlainText(text);
  ui->sourceTextEdit->document()->clearUndoRedoStacks();

  m_changed = false;

  refreshUi();
}

void
SourceEditorWindow::highlightError(
    std::string const &file,
    int line,
    int character,
    std::string const &error)
{
  if (file == m_fileName) {
    m_notifyingError = true;
    ui->statusbar->showMessage(
          QString::asprintf(
          "%s: line %d, character %d: %s",
          file.c_str(),
          line + 1,
          character,
          error.c_str()));
    m_highlighter->highlightError(line);
    QTextCursor cursor(ui->sourceTextEdit->document()->findBlockByLineNumber(line));
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, character - 1);
    ui->sourceTextEdit->setTextCursor(cursor);
    ui->sourceTextEdit->setFocus();
    m_notifyingError = false;
  }
}

SourceEditorParserContext *
SourceEditorWindow::makeParserContext(RZ::Recipe *recipe)
{
  return new SourceEditorParserContext(recipe, ui->sourceTextEdit);
}

SourceEditorWindow::~SourceEditorWindow()
{
  delete ui;
}

void
SourceEditorWindow::notifyChanged()
{
  if (!m_changed) {
    m_changed = true;
    refreshUi();
  }
}

/////////////////////////////////// Slots /////////////////////////////////////
void
SourceEditorWindow::onCursorChanged()
{
  QTextCursor cursor = ui->sourceTextEdit->textCursor();
  int y = cursor.blockNumber() + 1;
  int x = cursor.columnNumber() + 1;

  m_lineLabel->setText(QString::asprintf("Line: %d", y));
  m_colLabel->setText(QString::asprintf("Col: %d", x));
}

void
SourceEditorWindow::onTextEditChanged()
{
  if (!m_notifyingError) {
    m_highlighter->highlightError(-1);
    ui->statusbar->clearMessage();
    ui->actionUndo->setEnabled(ui->sourceTextEdit->document()->isUndoAvailable());
    ui->actionRedo->setEnabled(ui->sourceTextEdit->document()->isRedoAvailable());
    notifyChanged();
  }
}

void
SourceEditorWindow::onUndo()
{
  ui->sourceTextEdit->undo();
}

void
SourceEditorWindow::onRedo()
{
  ui->sourceTextEdit->redo();
}

void
SourceEditorWindow::onUndoAvailable(bool b)
{
  ui->actionUndo->setEnabled(b);
}

void
SourceEditorWindow::onRedoAvailable(bool b)
{
  ui->actionRedo->setEnabled(b);
}

void
SourceEditorWindow::onUndoAll()
{
  ui->sourceTextEdit->setPlainText(m_original.c_str());
  refreshUi();
}
