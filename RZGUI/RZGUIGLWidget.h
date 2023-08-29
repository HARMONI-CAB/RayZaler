#ifndef RZGUIGLWIDGET_H
#define RZGUIGLWIDGET_H

#include <QOpenGLWidget>
#include <OMModel.h>
#include <GLHelpers.h>

class RZGUIGLWidget : public QOpenGLWidget
{
  Q_OBJECT

  RZ::GLCappedCylinder m_axisCylinder;
  RZ::OMModel *m_model = nullptr;
  GLfloat m_viewPortMatrix[16];
  GLfloat m_refMatrix[16];

  bool    m_fixedLight = false;
  bool    m_newViewPort = false;
  int     m_width;
  int     m_height;
  int     m_hWnd = -1;
  GLfloat m_zoom = 1;

  bool    m_dragging = false;
  GLfloat m_dragStart[2] = {0, 0};

  GLfloat m_currentCenter[2] = {0, 0};
  GLfloat m_oldCenterCenter[2] = {0, 0};

  bool m_rotating = false;
  GLfloat m_rotStart[2] = {0, 0};
  GLfloat m_curRot[3] = {0, 0, 0};
  GLfloat m_oldRot[2] = {0, 0};

  void configureViewPort();
  void configureLighting();
  void pushElementMatrix(RZ::Element *);
  void popElementMatrix();
  void displayModel(RZ::OMModel *);

  void mouseClick(int button, int state, int x, int y, int shift);
  void mouseMotion(int x, int y);
  void drawAxes();

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
};

#endif // RZGUIGLWIDGET_H
