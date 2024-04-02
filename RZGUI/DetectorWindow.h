#ifndef DETECTORWINDOW_H
#define DETECTORWINDOW_H

#include <QMainWindow>
#include <list>

namespace Ui {
  class DetectorWindow;
}

namespace RZ {
  class Detector;
}

class QResizeEvent;
class ImageNavWidget;
class SimulationSession;

class DetectorWindow : public QMainWindow
{
  Q_OBJECT

  ImageNavWidget       *m_navWidget;
  RZ::Detector         *m_detector;
  SimulationSession    *m_session = nullptr;

  bool                  m_showPhotons = true;
  std::list<QAction *>  m_detectorActions;

  void populateDetectorMenu();
  void connectAll();
  void refreshUi();
  void refreshDetectorParams();

public:
  explicit DetectorWindow(QWidget *parent = nullptr);
  ~DetectorWindow() override;

  void setSession(SimulationSession *session);
  void setDetector(RZ::Detector *);
  void refreshImage();

  void closeEvent(QCloseEvent *) override;
  void showEvent(QShowEvent *) override;
  void resizeEvent(QResizeEvent *event) override;

public slots:
  void onScrollBarsChanged();
  void onViewChanged();
  void onClearDetector();
  void onToggleLogScale();
  void onChangeDetector();
  void onHoverPixel(QPointF loc);
  void onChangeDetectorRep();
  void onToggleShowPhase();

private:
  Ui::DetectorWindow *ui;
};

#endif // DETECTORWINDOW_H
