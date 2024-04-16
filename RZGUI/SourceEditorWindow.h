#ifndef SOURCEEDITORWINDOW_H
#define SOURCEEDITORWINDOW_H

#include <QMainWindow>
#include <cstdio>

namespace Ui {
  class SourceEditorWindow;
}

class RZMHighLighter;
class QLabel;

class SourceEditorWindow : public QMainWindow
{
  Q_OBJECT

  QLabel         *m_lineLabel = nullptr;
  QLabel         *m_colLabel  = nullptr;
  RZMHighLighter *m_highlighter = nullptr;

  void connectAll();

public:
  explicit SourceEditorWindow(QWidget *parent = nullptr);

  void loadFromFp(FILE *fp);

  ~SourceEditorWindow();

public slots:
  void onCursorChanged();

private:
  Ui::SourceEditorWindow *ui;
};

#endif // SOURCEEDITORWINDOW_H
