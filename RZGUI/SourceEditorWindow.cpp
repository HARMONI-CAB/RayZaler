#include "SourceEditorWindow.h"
#include "ui_SourceEditorWindow.h"
#include "RZMHighLighter.h"

SourceEditorWindow::SourceEditorWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::SourceEditorWindow)
{
  QFont font;
  font.setFamily("Cascadia Mono PL");
  font.setFixedPitch(true);
  font.setPointSize(10);

  ui->setupUi(this);

  ui->sourceTextEdit->setFont(font);

  m_highlighter = new RZMHighLighter(ui->sourceTextEdit->document());
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
