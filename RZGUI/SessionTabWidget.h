#ifndef SESSIONTABWIDGET_H
#define SESSIONTABWIDGET_H

#include <QWidget>

namespace Ui {
  class SessionTabWidget;
}

class SimulationSession;
class RZGUIGLWidget;
class SimulationProgressDialog;
class DetectorWindow;

class SessionTabWidget : public QWidget
{
  Q_OBJECT

  SimulationSession *m_session;  // Borrowed
  RZGUIGLWidget     *m_glWidget; // Borrowed

  DetectorWindow    *m_detWindow = nullptr; // Owned
  SimulationProgressDialog *m_progressDialog = nullptr; // Owned

  void connectAll();

public:
  explicit SessionTabWidget(SimulationSession *, QWidget *parent = nullptr);
  ~SessionTabWidget() override;

  SimulationSession *session() const;
  void showDetectorWindow();
  void updateDetectorWindow();
  void updateModel();


private:
  Ui::SessionTabWidget *ui;

public slots:
  void onModelChanged();
  void onSimulationTriggered(QString);
};

#endif // SESSIONTABWIDGET_H
