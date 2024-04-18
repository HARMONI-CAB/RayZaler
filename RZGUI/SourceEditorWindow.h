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
  std::string     m_original;
  bool            m_changed = false;

  void connectAll();
  void notifyChanged();

public:
  explicit SourceEditorWindow(QWidget *parent = nullptr);

  void refreshUi();
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
  void onUndoAll();
  void onUndo();
  void onRedo();
  void onUndoAvailable(bool);
  void onRedoAvailable(bool);

private:
  Ui::SourceEditorWindow *ui;
};

#endif // SOURCEEDITORWINDOW_H
