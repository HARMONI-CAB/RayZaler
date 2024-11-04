#ifndef SPOTDIAGRAMWINDOW_H
#define SPOTDIAGRAMWINDOW_H

#include <QMainWindow>

namespace Ui {
  class SpotDiagramWindow;
}

class SpotDiagramWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit SpotDiagramWindow(QWidget *parent = nullptr);
  ~SpotDiagramWindow();

private:
  Ui::SpotDiagramWindow *ui;
};

#endif // SPOTDIAGRAMWINDOW_H
