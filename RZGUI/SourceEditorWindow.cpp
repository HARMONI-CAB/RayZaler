#include "SourceEditorWindow.h"
#include "ui_SourceEditorWindow.h"
#include "RZMHighLighter.h"
#include <QLabel>

SourceEditorWindow::SourceEditorWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::SourceEditorWindow)
{
  QFont font;
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

  m_highlighter = new RZMHighLighter(ui->sourceTextEdit->document());

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

  while (!feof(fp)) {
    size = fread(buffer.data(), sizeof(char), buffer.size() - 1, fp);
    if (size > 0) {
      buffer.data()[size] = 0;
      text += buffer.data();
    }
  }

  fseek(fp, curr, SEEK_SET);

  ui->sourceTextEdit->setPlainText(text);
}

SourceEditorWindow::~SourceEditorWindow()
{
  delete ui;
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
