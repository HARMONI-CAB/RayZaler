#ifndef SESSIONTABWIDGET_H
#define SESSIONTABWIDGET_H

#include <QWidget>

namespace Ui {
  class SessionTabWidget;
}

namespace RZ {
  class ReferenceFrame;
  class OpticalPath;
}

class SimulationSession;
class RZGUIGLWidget;
class SimulationProgressDialog;
class DetectorWindow;
class SourceEditorWindow;

class SessionTabWidget : public QWidget
{
  Q_OBJECT

  SimulationSession        *m_session;  // Borrowed
  RZGUIGLWidget            *m_glWidget; // Borrowed
  DetectorWindow           *m_detWindow = nullptr; // Owned
  SimulationProgressDialog *m_progressDialog = nullptr; // Owned
  SourceEditorWindow       *m_sourceEditorWindow = nullptr;
  bool                      m_displayNames     = false;
  bool                      m_displayApertures = false;
  bool                      m_displayElements  = true;
  bool                      m_displayRefFrames = false;

  void connectAll();

public:
  explicit SessionTabWidget(SimulationSession *, QWidget *parent = nullptr);
  ~SessionTabWidget() override;

  SimulationSession *session() const;
  void clearBeam();
  void showDetectorWindow();
  void updateDetectorWindow();
  void updateModel();
  void reloadModel();
  void setRotation(qreal, qreal, qreal);
  void showSourceWindow();
  bool displayNames() const;
  bool displayApertures() const;
  bool displayElements() const;
  bool displayRefFrames() const;
  void setDisplayNames(bool);
  void setDisplayApertures(bool);
  void setDisplayElements(bool);
  void setDisplayRefFrames(bool);
  void setSelectedReferenceFrame(RZ::ReferenceFrame *);
  void setSelectedOpticalPath(const RZ::OpticalPath *);

  void keyPressEvent(QKeyEvent *event) override;

private:
  Ui::SessionTabWidget *ui;

public slots:
  void onModelChanged();
  void onSimulationTriggered(QString, int, int);
  void onSweepFinished();
};

#endif // SESSIONTABWIDGET_H
