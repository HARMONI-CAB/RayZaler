#ifndef RZGUIGLWIDGET_H
#define RZGUIGLWIDGET_H

#include <QOpenGLWidget>
#include <OMModel.h>
#include <GLHelpers.h>
#include <IncrementalRotation.h>
#include <QKeyEvent>

class RZGUIGLWidget : public QOpenGLWidget
{
  Q_OBJECT

  RZ::GLReferenceFrame      m_glAxes;
  RZ::GLGrid                m_xyCoarseGrid;
  RZ::GLGrid                m_xyMediumGrid;
  RZ::GLGrid                m_xyFineGrid;
  RZ::GLText                m_glGridText;
  RZ::GLText                m_glLabelText;

  std::vector<RZ::GLArrow>  m_pathArrows;
  const RZ::ReferenceFrame *m_selectedRefFrame = nullptr;
  const RZ::OpticalPath    *m_selectedPath = nullptr;

  RZ::OMModel *m_model = nullptr;
  GLfloat m_viewPortMatrix[16];
  GLfloat m_refMatrix[16];

  bool    m_displayNames        = false;
  bool    m_displayApertures    = false;
  bool    m_displayElements     = true;
  bool    m_displayRefFrames    = false;
  bool    m_displayGrids        = true;
  bool    m_displayMeasurements = false;

  bool    m_fixedLight          = false;
  bool    m_newViewPort         = false;
  int     m_width;
  int     m_height;
  int     m_hWnd = -1;
  GLfloat m_zoom = 1;
  bool    m_dragging = false;
  GLfloat m_dragStart[2] = {0, 0};

  GLfloat m_currentCenter[2] = {0, 0};
  GLfloat m_oldCenterCenter[2] = {0, 0};

  bool m_rotating = false;
  RZ::IncrementalRotation m_incRot;
  GLfloat m_prevRotX, m_prevRotY;
  GLfloat m_rotStart[2] = {0, 0};
  GLfloat m_curAzEl[3] = {0, 0};
  GLfloat m_oldRot[2] = {0, 0};

  void configureViewPort();
  void configureLighting();
  void setOrientationAndCenter(RZ::Matrix3 const &, RZ::Vec3 const &);
  void pushReferenceFrameMatrix(const RZ::ReferenceFrame *);
  void pushElementMatrix(const RZ::Element *);
  void popElementMatrix();
  void displayModel(RZ::OMModel *);
  void displayApertures(const RZ::Element *);
  void mouseClick(int button, int state, int x, int y, int shift);
  void mouseMotion(int x, int y);

  void displayAxes();
  void displayBeam(RZ::RayBeamElement *, bool dynamicAlpha);
  void displayCurrentGrid();
  void displayCurrentPath();
  void displayCurrentRefFrame();

  void displayLoop(
    RZ::OMModel *model,
    bool elements,
    bool apertures,
    bool frames,
    bool labels);

  void drawAxes();

  inline GLint project(
      GLdouble objx,
      GLdouble objy,
      GLdouble objz,
      const GLdouble model[16],
      const GLdouble proj[16],
      const GLint viewport[4],
      GLdouble *winx,
      GLdouble *winy,
      GLdouble *winz);

  inline void transformPoint(
      GLdouble out[4],
      const GLdouble m[16],
      const GLdouble in[4]);

  void renderText(
      qreal x,
      qreal y,
      qreal z,
      const QString &str,
      const QColor & color = QColor(Qt::cyan),
      const QFont & font = QFont());

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  void mouseMoveEvent(QMouseEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void wheelEvent(QWheelEvent *) override;

public:
  RZGUIGLWidget(QWidget *);
  void setModel(RZ::OMModel *model);
  void getCurrentRot(GLfloat *) const;
  void setCurrentRot(const GLfloat *);
  void setDisplayNames(bool);
  void setDisplayApertures(bool);
  void setDisplayElements(bool);
  void setDisplayRefFrames(bool);
  void setDisplayMeasurements(bool);
  void setDisplayGrid(bool);
  
  void setSelectedOpticalPath(RZ::OpticalPath const *path);
  void setSelectedReferenceFrame(RZ::ReferenceFrame *sel, const char *name = nullptr);
  void rotateToCurrentFrame();

  void setGridDivs(unsigned);
  void setGridStep(qreal);

  // Exposed so other objects can deliver events to the widget
  void keyPressEvent(QKeyEvent *event) override;

signals:
  void planeCoords(qreal x, qreal y);
};

#endif // RZGUIGLWIDGET_H
