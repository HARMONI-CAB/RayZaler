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
  const RZ::ReferenceFrame *m_selectedFrame    = nullptr;
  bool                      m_displayNames     = false;
  bool                      m_displayApertures = false;
  bool                      m_displayElements  = true;
  bool                      m_displayRefFrames = false;
  bool                      m_displayGrid      = true;

  void connectAll();
  void addGridStep(QString const &, qreal);
  void addGridDiv(unsigned);

public:
  explicit SessionTabWidget(SimulationSession *, QWidget *parent = nullptr);
  ~SessionTabWidget() override;

  SimulationSession *session() const;
  void clearBeam();
  void showDetectorWindow();
  void updateDetectorWindow();
  void updateModel();
  void reloadModel();
  void reloadModelFromEditor();
  void setRotation(qreal, qreal, qreal);
  void showSourceWindow();
  void centerSelectedFrame();

  bool displayNames() const;
  bool displayApertures() const;
  bool displayElements() const;
  bool displayRefFrames() const;
  bool displayGrid() const;
  const RZ::ReferenceFrame *selectedFrame() const;

  void setDisplayNames(bool);
  void setDisplayApertures(bool);
  void setDisplayElements(bool);
  void setDisplayRefFrames(bool);
  void setDisplayGrid(bool);

  void setSelectedReferenceFrame(RZ::ReferenceFrame *, const char * = nullptr);
  void setSelectedOpticalPath(const RZ::OpticalPath *);

  void setGridStep(qreal);
  void setGridDivs(unsigned);

  void keyPressEvent(QKeyEvent *event) override;

private:
  Ui::SessionTabWidget *ui;

public slots:
  void onModelChanged();
  void onSimulationTriggered(QString, int, int);
  void onSweepFinished();
  void onSourceEditorBuild();
  void onGridStepChanged(int);
  void onGridDivChanged(int);
  void onNewCoords(qreal, qreal);
};

#endif // SESSIONTABWIDGET_H
