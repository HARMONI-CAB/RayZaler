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
class FootprintInfoWidget;

struct SurfaceFootprint {
  std::string           fullName;     // Full surface name
  std::string           label;        // Label as shown in the plot
  std::vector<qreal>    locations;    // Location array
  std::vector<qreal>    directions;   // Location array
  uint32_t              color;        // Color for representation
  size_t                transmitted;  // Transmitted rays
  size_t                vignetted;    // Vignetted
};

class SpotDiagramWindow : public QMainWindow
{
  Q_OBJECT

  RZ::ScatterDataProduct           *m_product = nullptr;
  ScatterWidget                    *m_widget  = nullptr;
  QFileDialog                      *m_saveDialog = nullptr;
  std::list<FootprintInfoWidget *>  m_infoWidgets;
  int                               m_legendWidth = 250;
  void connectAll();

public:
  virtual void resizeEvent(QResizeEvent *ev) override;

  explicit SpotDiagramWindow(QString path, QWidget *parent = nullptr);
  virtual ~SpotDiagramWindow() override;
  void updateView();
  void transferFootprint(SurfaceFootprint &);
  void setEdges(std::vector<std::vector<RZ::Real>> const &);


public slots:
  void onSaveData();
  void onClear();
  void resetZoom();
  void onSplitterMoved();

signals:
  void clear();
  void saveData(QString);

private:
  Ui::SpotDiagramWindow *ui;
};

#endif // SPOTDIAGRAMWINDOW_H
