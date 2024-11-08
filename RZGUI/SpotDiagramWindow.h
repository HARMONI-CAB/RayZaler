#ifndef SPOTDIAGRAMWINDOW_H
#define SPOTDIAGRAMWINDOW_H

#include <QMainWindow>
#include <Vector.h>

class QFileDialog;

namespace RZ {
  class ScatterDataProduct;
}

namespace Ui {
  class SpotDiagramWindow;
}

class ScatterWidget;

struct SurfaceFootprint {
  std::string           fullName;  // Full surface name
  std::string           label;     // Label as shown in the plot
  std::vector<qreal> locations;    // Location array
  uint32_t              color;     // Color for representation
};

class SpotDiagramWindow : public QMainWindow
{
  Q_OBJECT

  RZ::ScatterDataProduct *m_product = nullptr;
  ScatterWidget          *m_widget  = nullptr;
  QFileDialog            *m_saveDialog = nullptr;

  void legend();
  void connectAll();

public:
  explicit SpotDiagramWindow(QString path, QWidget *parent = nullptr);
  virtual ~SpotDiagramWindow() override;
  void updateView();
  void transferFootprint(SurfaceFootprint &);
  void setEdges(std::vector<std::vector<RZ::Real>> const &);


public slots:
  void onSaveData();
  void onClear();
  void resetZoom();

signals:
  void clear();
  void saveData(QString);

private:
  Ui::SpotDiagramWindow *ui;
};

#endif // SPOTDIAGRAMWINDOW_H
