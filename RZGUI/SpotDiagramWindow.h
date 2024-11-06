#ifndef SPOTDIAGRAMWINDOW_H
#define SPOTDIAGRAMWINDOW_H

#include <QMainWindow>

namespace RZ {
  class ScatterDataProduct;
}

namespace Ui {
  class SpotDiagramWindow;
}

class ScatterWidget;
class SpotDiagramWindow : public QMainWindow
{
  Q_OBJECT

  RZ::ScatterDataProduct *m_product = nullptr;
  ScatterWidget          *m_widget  = nullptr;

  void legend();

public:
  explicit SpotDiagramWindow(RZ::ScatterDataProduct *, QWidget *parent = nullptr);
  virtual ~SpotDiagramWindow() override;
  void updateView();

private:
  Ui::SpotDiagramWindow *ui;
};

#endif // SPOTDIAGRAMWINDOW_H
