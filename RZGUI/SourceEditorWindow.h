#ifndef SOURCEEDITORWINDOW_H
#define SOURCEEDITORWINDOW_H

#include <QMainWindow>
#include <cstdio>

namespace Ui {
  class SourceEditorWindow;
}

class RZMHighLighter;

class SourceEditorWindow : public QMainWindow
{
  Q_OBJECT

  RZMHighLighter *m_highlighter = nullptr;

public:
  explicit SourceEditorWindow(QWidget *parent = nullptr);

  void loadFromFp(FILE *fp);

  ~SourceEditorWindow();

private:
  Ui::SourceEditorWindow *ui;
};

#endif // SOURCEEDITORWINDOW_H
