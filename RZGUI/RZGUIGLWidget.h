//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

#ifndef RZGUIGLWIDGET_H
#define RZGUIGLWIDGET_H

#include <QOpenGLWidget>
#include <OMModel.h>
#include <GLHelpers.h>
#include <IncrementalRotation.h>
#include <QKeyEvent>
#include <GLModel.h>
#include <GLRenderEngine.h>

class RZGUIGLWidget : public QOpenGLWidget
{
  Q_OBJECT

  RZ::GLHelperGrid          m_grid;
  RZ::GLReferenceFrame      m_glAxes;
  RZ::GLText                m_glLabelText;
  RZ::GLCurrentView         m_view;

  std::vector<RZ::GLArrow>  m_pathArrows;
  const RZ::ReferenceFrame *m_selectedRefFrame = nullptr;
  const RZ::OpticalPath    *m_selectedPath = nullptr;
  RZ::Element              *m_selectedElement = nullptr;

  RZ::OMModel *m_model = nullptr;
  GLfloat m_refMatrix[16];

  bool    m_displayNames        = false;
  bool    m_displayApertures    = false;
  bool    m_displayElements     = true;
  bool    m_displayRefFrames    = false;
  bool    m_displayGrids        = true;
  bool    m_displayMeasurements = false;

  bool    m_zoomToBoxPending    = false;

  RZ::Vec3 m_lastP1, m_lastP2;

  bool    m_fixedLight          = false;
  bool    m_newViewPort         = false;
  int     m_width               = 0;
  int     m_height              = 0;
  int     m_hWnd = -1;
  bool    m_dragging = false;

  GLfloat m_bgAbove[3] = {1.f / 255.f, 1.f / 255.f, 1.f / 255.f};
  GLfloat m_bgBelow[3] = {0x75f / 255.f, 0x75f / 255.f, 0xe9f / 255.f};

  GLfloat m_pathColor[3] = {1., 0, 1.};
  GLfloat m_gridColor[3] = {1., 1., 1.};

  GLfloat m_dragStart[2] = {0, 0};
  GLfloat m_oldCenterCenter[2] = {0, 0};

  bool m_rotating = false;
  GLfloat m_prevRotX, m_prevRotY;
  GLfloat m_rotStart[2] = {0, 0};
  GLfloat m_curAzEl[3] = {0, 0};
  GLfloat m_oldRot[2] = {0, 0};

  qreal m_gridStep = 1e-3;
  unsigned m_gridDivs = 100;

  void configureViewPort();
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
  
  RZ::GLCurrentView *view();

  void setModel(RZ::OMModel *model);
  void zoomToBox(RZ::Vec3 const &, RZ::Vec3 const &);
  void getCurrentRot(GLfloat *) const;
  void setCurrentRot(const GLfloat *);
  void setDisplayNames(bool);
  void setDisplayApertures(bool);
  void setDisplayElements(bool);
  void setDisplayRefFrames(bool);
  void setDisplayMeasurements(bool);
  void setDisplayGrid(bool);
  void setBackgroundGradient(const GLfloat *, const GLfloat *);
  void setPathColor(const GLfloat *);
  void setGridColor(const GLfloat *);
  void setSelectedOpticalPath(RZ::OpticalPath const *path);
  void setSelectedReferenceFrame(RZ::ReferenceFrame *sel, const char *name = nullptr);
  void setSelectedElement(RZ::Element *);
  void rotateToCurrentFrame();

  void setGridDivs(unsigned);
  void setGridStep(qreal);
  unsigned gridDivs() const;
  qreal gridStep() const;

  void display();
  void configureLighting();

  // Exposed so other objects can deliver events to the widget
  void keyPressEvent(QKeyEvent *event) override;

signals:
  void planeCoords(qreal x, qreal y);
};

#endif // RZGUIGLWIDGET_H
