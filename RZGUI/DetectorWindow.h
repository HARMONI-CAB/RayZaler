#ifndef DETECTORWINDOW_H
#define DETECTORWINDOW_H

#include <QMainWindow>

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

  ImageNavWidget    *m_navWidget;
  RZ::Detector      *m_detector;
  SimulationSession *m_session = nullptr;

  void connectAll();
  void refreshUi();

public:
  explicit DetectorWindow(QWidget *parent = nullptr);
  ~DetectorWindow() override;

  void setSession(SimulationSession *session);
  void refreshImage();

  void closeEvent(QCloseEvent *) override;
  void showEvent(QShowEvent *) override;
  void resizeEvent(QResizeEvent *event) override;

public slots:
  void onScrollBarsChanged();
  void onViewChanged();
  void onClearDetector();
  void onToggleLogScale();

private:
  Ui::DetectorWindow *ui;
};

#endif // DETECTORWINDOW_H
