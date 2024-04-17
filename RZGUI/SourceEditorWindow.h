#ifndef SOURCEEDITORWINDOW_H
#define SOURCEEDITORWINDOW_H

#include <QMainWindow>
#include <cstdio>
#include <ParserContext.h>

namespace Ui {
  class SourceEditorWindow;
}

class RZMHighLighter;
class QLabel;
class QTextEdit;

class SourceEditorParserContext : public RZ::ParserContext {
  std::string m_source;
  int m_pos = 0;

public:
  SourceEditorParserContext(RZ::Recipe *recipe, QTextEdit *edit);
  virtual int read() override;
};

class SourceEditorWindow : public QMainWindow
{
  Q_OBJECT

  QLabel         *m_lineLabel = nullptr;
  QLabel         *m_colLabel  = nullptr;
  std::string     m_fileName;

  bool            m_notifyingError = false;
  RZMHighLighter *m_highlighter = nullptr;

  void connectAll();

public:
  explicit SourceEditorWindow(QWidget *parent = nullptr);

  void loadFromFp(FILE *fp);
  void setFileName(std::string const &);
  SourceEditorParserContext *makeParserContext(RZ::Recipe *);
  void highlightError(std::string const &, int, int, std::string const &);
  ~SourceEditorWindow() override;

signals:
  void build();

public slots:
  void onCursorChanged();
  void onTextEditChanged();

private:
  Ui::SourceEditorWindow *ui;
};

#endif // SOURCEEDITORWINDOW_H
