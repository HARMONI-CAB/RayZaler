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
  bool               m_displayNames     = false;
  bool               m_displayApertures = false;
  bool               m_displayElements  = true;

  void connectAll();

public:
  explicit SessionTabWidget(SimulationSession *, QWidget *parent = nullptr);
  ~SessionTabWidget() override;

  SimulationSession *session() const;
  void showDetectorWindow();
  void updateDetectorWindow();
  void updateModel();
  void setRotation(qreal, qreal, qreal);
  bool displayNames() const;
  bool displayApertures() const;
  bool displayElements() const;
  void setDisplayNames(bool);
  void setDisplayApertures(bool);
  void setDisplayElements(bool);

  void keyPressEvent(QKeyEvent *event) override;

private:
  Ui::SessionTabWidget *ui;

public slots:
  void onModelChanged();
  void onSimulationTriggered(QString, int, int);
  void onSweepFinished();
};

#endif // SESSIONTABWIDGET_H
