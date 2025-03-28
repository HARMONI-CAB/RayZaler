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

#include "RZGUIGLWidget.h"
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QWheelEvent>
#include <GLHelpers.h>
#include <GL/glut.h>
#include <QPainter>
#include "GUIHelpers.h"
#include <SurfaceShape.h>
#include <Elements/RayBeamElement.h>
#include <QKeyEvent>

#define RZGUIGL_MOUSE_ROT_DELTA 2e-1
#define RZGUIGL_KBD_ROT_DELTA   5

RZGUIGLWidget::RZGUIGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);

  m_glLabelText.setFace("gridfont");

  RZ::IncrementalRotation screenRotation;

  screenRotation.rotateRelative(RZ::Vec3::eX(), -M_PI / 2);
  screenRotation.rotateRelative(RZ::Vec3::eZ(), -M_PI / 2);

  m_view.setScreenRotation(screenRotation);
}

void
RZGUIGLWidget::setOrientationAndCenter(RZ::Matrix3 const &R, RZ::Vec3 const &O)
{
   GLdouble viewMatrix[16] = {
    R.rows[0].coords[0], R.rows[1].coords[0], R.rows[2].coords[0], 0,
    R.rows[0].coords[1], R.rows[1].coords[1], R.rows[2].coords[1], 0,
    R.rows[0].coords[2], R.rows[1].coords[2], R.rows[2].coords[2], 0,
            O.coords[0],         O.coords[1],         O.coords[2], 1
   };

  glMultMatrixd(viewMatrix);
}

void
RZGUIGLWidget::pushReferenceFrameMatrix(const RZ::ReferenceFrame *frame)
{
  auto R     = frame->getOrientation();
  auto O     = frame->getCenter();

  glPushMatrix();
  glLoadMatrixf(m_refMatrix);
  setOrientationAndCenter(R, O);
}

void
RZGUIGLWidget::setGridDivs(unsigned num)
{
  m_gridDivs = num;
  m_grid.setGridDivs(num);
  update();
}

void
RZGUIGLWidget::setGridStep(qreal step)
{
  m_gridStep = step;
  m_grid.setGridStep(step);
  update();
}

qreal
RZGUIGLWidget::gridStep() const
{
  return m_gridStep;
}

unsigned int
RZGUIGLWidget::gridDivs() const
{
  return m_gridDivs;
}


void
RZGUIGLWidget::pushElementMatrix(const RZ::Element *element)
{
  pushReferenceFrameMatrix(element->parentFrame());
}

void
RZGUIGLWidget::popElementMatrix()
{
  glPopMatrix();
}

inline void
RZGUIGLWidget::transformPoint(
    GLdouble out[4],
    const GLdouble m[16],
    const GLdouble in[4])
{
#define M(row,col)  m[col*4+row]
    out[0] =
        M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
    out[1] =
        M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
    out[2] =
        M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
    out[3] =
        M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

inline GLint
RZGUIGLWidget::project(
    GLdouble objx,
    GLdouble objy,
    GLdouble objz,
    const GLdouble model[16],
    const GLdouble proj[16],
    const GLint viewport[4],
    GLdouble *winx,
    GLdouble *winy,
    GLdouble *winz)
{
    GLdouble in[4], out[4];

    in[0] = objx;
    in[1] = objy;
    in[2] = objz;
    in[3] = 1.0;

    transformPoint(out, model, in);
    transformPoint(in, proj, out);

    if (in[3] == 0.0)
        return GL_FALSE;

    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];

    *winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
    *winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;

    *winz = (1 + in[2]) / 2;
    return GL_TRUE;
}

void
RZGUIGLWidget::renderText(
    qreal x,
    qreal y,
    qreal z,
    const QString &str,
    const QColor &color,
    const QFont &font) {
  GLdouble textPosX = 0, textPosY = 0, textPosZ = 0;
  QFontMetrics metrics(font);
  GLdouble model[4][4], proj[4][4];
  GLint view[4];
  int tw;

  glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glGetDoublev(GL_MODELVIEW_MATRIX, &model[0][0]);
  glGetDoublev(GL_PROJECTION_MATRIX, &proj[0][0]);
  glGetIntegerv(GL_VIEWPORT, &view[0]);

  project(
        x,            y,           z,
        &model[0][0], &proj[0][0], &view[0],
        &textPosX,    &textPosY,   &textPosZ);

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
  tw = metrics.horizontalAdvance(str);
#else
  tw = metrics.width(str);
#endif // QT_VERSION_CHECK

  textPosY = height() - textPosY + metrics.height() / 2; // y is inverted
  textPosX -= tw / 2;

  // Render text
  QPainter painter(this);
  QPen pen(color);
  painter.setPen(pen);
  painter.setFont(font);
  painter.drawText(TOINT(textPosX), TOINT(textPosY), str);
  painter.end();

  glPopAttrib();
}

void
RZGUIGLWidget::displayApertures(const RZ::Element *el)
{
  if (el->hasProperty("optical")) {
    glPushAttrib(GL_LINE_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    if (m_displayElements)
      glColor3f(0, 0, 1);
    else
      glColor3f(1, 1, 1);

    glLineWidth(2);
    const RZ::OpticalElement *optEl = static_cast<const RZ::OpticalElement *>(el);
    auto &surfaces = optEl->opticalSurfaces();
    
    for (auto &surf : surfaces) {
      RZ::SurfaceShape *ap = surf->boundary->surfaceShape();

      if (ap != nullptr) {
        pushReferenceFrameMatrix(surf->frame);
        
        ap->renderOpenGL();

        if (el == m_selectedElement) {
          glColor3f(1, 1, 0);
          
          ap->renderOpenGLExtra();

          if (m_displayElements)
            glColor3f(0, 0, 1);
          else
            glColor3f(1, 1, 1);
        }
        
        /*if (!surf->boundary->infinite()) {
          
        }*/

        glPopMatrix();
      }
    }
    glPopAttrib();
  }
}

void
RZGUIGLWidget::displayBeam(RZ::RayBeamElement *beam, bool dynAlpha)
{
  beam->setDynamicAlpha(dynAlpha);

  pushElementMatrix(beam);
  beam->renderOpenGL();
  popElementMatrix();
}

void
RZGUIGLWidget::displayLoop(
  RZ::OMModel *model,
  bool elements,
  bool apertures,
  bool frames,
  bool labels)
{
  RZ::RayBeamElement *beam = static_cast<RZ::RayBeamElement *>(model->beam());

  for (auto p : model->elementList()) {
    if (p == beam || !p->visible())
      continue;
    
    pushElementMatrix(p);
    // vvvvvvvvvvvvvvvvvvvv BEGIN: Display at p's frame vvvvvvvvvvvvvvvvvvvv
    
    if (labels) {
      QString str = QString::fromStdString(p->name());
      renderText(0, 0, 0, str);
    }

    if (frames)
      displayAxes();

    if (p == m_selectedElement)
      p->renderBoundingBoxOpenGL();

    if (elements)
      p->renderOpenGL();

    if (apertures)
      displayApertures(p);

    // ^^^^^^^^^^^^^^^^^^^^^ END: Display at p's frame ^^^^^^^^^^^^^^^^^^^^^
    popElementMatrix();

    if (p->nestedModel() != nullptr)
      displayModel(p->nestedModel());
  }
}

void
RZGUIGLWidget::displayCurrentRefFrame()
{
  pushReferenceFrameMatrix(m_selectedRefFrame);
  if (m_displayGrids)
    m_grid.display();
  
  displayAxes();
  glPopMatrix();
}

void
RZGUIGLWidget::displayModel(RZ::OMModel *model)
{
  bool dynAlpha = m_displayElements && false;

  if (dynAlpha)
    displayLoop(
      model,
      true,   // Elements
      false,  // Apertures
      false,  // Frames
      false); // Labels

  if (model == m_model) {
    RZ::RayBeamElement *beam = static_cast<RZ::RayBeamElement *>(model->beam());
    displayBeam(beam, dynAlpha);
  }

  displayLoop(
    model,
    m_displayElements,
    m_displayApertures,
    m_displayRefFrames,
    m_displayNames);

  // The following elements are displayed only in the top-level model
  if (model == m_model) {
    if (m_selectedRefFrame != nullptr)
      displayCurrentRefFrame();

    if (!m_pathArrows.empty())
      displayCurrentPath();
  }
}

void
RZGUIGLWidget::displayAxes()
{
  auto zoom = m_view.zoomLevel;

  glPushMatrix();
  glScalef(1. / zoom, 1. / zoom, 1. / zoom);
  m_glAxes.display();
  glPopMatrix();
}

void
RZGUIGLWidget::displayCurrentPath()
{
  auto path = m_selectedPath;

  if (path != nullptr) {
    glColor3fv(m_pathColor);
    unsigned int i = 0;
    for (auto p = path->m_sequence.begin();
          p != path->m_sequence.end();
          ++p) {
        auto q = std::next(p);
      if (q == path->m_sequence.end())
        break;

      m_pathArrows[i++].display();
    }
  }
}

void
RZGUIGLWidget::setDisplayNames(bool state)
{
  if (m_displayNames != state) {
    m_displayNames = state;    
    update();
  }
}

void
RZGUIGLWidget::setDisplayRefFrames(bool state)
{
  if (m_displayRefFrames != state) {
    m_displayRefFrames = state;
    update();
  }
}

void
RZGUIGLWidget::setDisplayGrid(bool state)
{
  if (m_displayGrids != state) {
    m_displayGrids = state;
    update();
  }
}

void
RZGUIGLWidget::setBackgroundGradient(const GLfloat *above, const GLfloat *below)
{
  memcpy(m_bgAbove, above, 3 * sizeof(GLfloat));
  memcpy(m_bgBelow, below, 3 * sizeof(GLfloat));
  update();
}

void
RZGUIGLWidget::setGridColor(const GLfloat *color)
{
  memcpy(m_gridColor, color, 3 * sizeof(GLfloat));
  m_grid.setColor(m_gridColor);
  update();
}

void
RZGUIGLWidget::setPathColor(const GLfloat *color)
{
  memcpy(m_pathColor, color, 3 * sizeof(GLfloat));
  update();
}

void
RZGUIGLWidget::setDisplayMeasurements(bool state)
{
  if (state != m_displayMeasurements) {
    m_displayMeasurements = state;

    if (!state)
      m_grid.highlight(0, 0);
    
    update();
  }
}

void
RZGUIGLWidget::setDisplayApertures(bool state)
{
  if (m_displayApertures != state) {
    m_displayApertures = state;
    update();
  }
}

void
RZGUIGLWidget::setDisplayElements(bool state)
{
  if (m_displayElements != state) {
    m_displayElements = state;
    update();
  }
}

void
RZGUIGLWidget::setSelectedOpticalPath(const RZ::OpticalPath *path)
{
  if (m_selectedPath != path) {
    m_selectedPath = path;

    m_pathArrows.clear();

    if (path != nullptr) {
      auto p = path->m_sequence.begin();
      for (auto q = std::next(p);
          q != path->m_sequence.end();
          p = q, q = std::next(p)) {
        RZ::Vec3 qCenter = (*q)->parent->getVertex();
        RZ::Vec3 pCenter = (*p)->parent->getVertex();

        RZ::GLArrow arrow;

        arrow.setOrigin(pCenter);
        arrow.setDirection(qCenter - pCenter);
        arrow.setThickness(4);
        m_pathArrows.push_back(arrow);
      }

      update();
    }
  }
}

void
RZGUIGLWidget::setSelectedReferenceFrame(RZ::ReferenceFrame *frame, const char *name)
{
  if (m_selectedRefFrame != frame) {
    if (frame != nullptr) {
      if (name ==  nullptr)
        name = frame->name().c_str();
      m_grid.setGridText(name);
    }
    
    m_selectedRefFrame = frame;
    
    update();
  }
}

void
RZGUIGLWidget::setSelectedElement(RZ::Element *el)
{
  m_selectedElement = el;

  if (el != nullptr)
    el->calcBoundingBoxOpenGL();
}

void
RZGUIGLWidget::configureLighting()
{
  RZ::GLVectorStorage vec;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glLightfv(GL_LIGHT0, GL_AMBIENT,  vec.get(1.0, 1.0, 1.0));
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  vec.get(1.0, 1.0, 1.0));
  glLightfv(GL_LIGHT0, GL_SPECULAR, vec.get(1.0, 1.0, 1.0));
  glEnable(GL_LIGHT0);

  glShadeModel(GL_SMOOTH);
  glCullFace(GL_BACK);
}

void
RZGUIGLWidget::mouseClick(int button, int state, int x, int y, int shift)
{
  RZ::Real wX, wY;

  m_view.screenToWorld(wX, wY, x, y);

  if (state == 1) {
    // Released
    switch (button) {
      case 0:
        m_dragging = false;
        break;

      case 2:
        m_rotating = false;
        break;
    }
  } else if (state == 0) {
    bool newZoom = false;
    // Clicked

    switch (button) {
      case 0: // Left
        m_dragging = true;
        m_dragStart[0] = x;
        m_dragStart[1] = y;
        m_oldCenterCenter[0] = m_view.center[0];
        m_oldCenterCenter[1] = m_view.center[1];

        break;

      case 2: // Right button
        m_rotating = true;
        m_rotStart[0] = m_prevRotX = x;
        m_rotStart[1] = m_prevRotY = y;
        m_oldRot[0] = m_curAzEl[0];
        m_oldRot[1] = m_curAzEl[1];
        break;

      case 3: // Mouse wheel up
        m_view.zoom(pow(1.1, +shift / 120.));
        newZoom = true;
        break;

      case 4: // Mouse wheel down
        m_view.zoom(pow(1.1, -shift / 120.));
        newZoom = true;
        break;
    }

    if (newZoom) {
      RZ::Real prevSX, prevSY;
      RZ::Real dX, dY;

      m_view.worldToScreen(prevSX, prevSY, wX, wY);

      dX = x - prevSX;
      dY = y - prevSY;

      m_view.move(-dX, -dY);
    }
  }

  m_newViewPort = true;
  update();
}

void
RZGUIGLWidget::mouseMotion(int x, int y)
{
  GLfloat shiftX, shiftY;
  RZ::Vec3 v;

  RZ::Real wX, wY;

  m_view.screenToWorld(wX, wY, x, y);

  RZ::Vec3 result;

  if (m_selectedRefFrame != nullptr && m_displayMeasurements) {
    if (m_view.worldToFrame(result, *m_selectedRefFrame, wX, wY)) {
      m_grid.highlight(result.x, result.y);
      emit planeCoords(result.x, result.y);
      update();
    } else {
      m_grid.highlight(0, 0);
    }
  } else {
    m_grid.highlight(0, 0);
  }
  
  if (m_dragging) {
    shiftX = x - m_dragStart[0];
    shiftY = y - m_dragStart[1];
    m_view.setCenter(
      m_oldCenterCenter[0] + shiftX,
      m_oldCenterCenter[1] + shiftY);

    m_newViewPort = true;

    update();
  }

  if (m_rotating) {
    shiftX = x - m_rotStart[0];
    shiftY = y - m_rotStart[1];
    m_curAzEl[0] = m_oldRot[0] + shiftX * RZGUIGL_MOUSE_ROT_DELTA;
    m_curAzEl[1] = m_oldRot[1] + shiftY * RZGUIGL_MOUSE_ROT_DELTA;

    RZ::Real deltaX = x - m_prevRotX;
    RZ::Real deltaY = y - m_prevRotY;


    m_view.incAzEl(
      deltaX * RZGUIGL_MOUSE_ROT_DELTA,
      deltaY * RZGUIGL_MOUSE_ROT_DELTA);

    m_newViewPort = true;
    
    m_prevRotX = x;
    m_prevRotY = y;

    update();
  }
}

void
RZGUIGLWidget::mouseMoveEvent(QMouseEvent *event)
{
  int x = static_cast<int>(event->position().x());
  int y = static_cast<int>(event->position().y());

  mouseMotion(x, y);
}

void
RZGUIGLWidget::mousePressEvent(QMouseEvent *event)
{
  int x = static_cast<int>(event->position().x());
  int y = static_cast<int>(event->position().y());

  switch (event->button()) {
    case Qt::LeftButton:
      mouseClick(0, 0, x, y, 0);
      break;

    case Qt::RightButton:
      mouseClick(2, 0, x, y, 0);
      break;

    default:
      break;
  }
}

void
RZGUIGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
  int x = static_cast<int>(event->position().x());
  int y = static_cast<int>(event->position().y());

  switch (event->button()) {
    case Qt::LeftButton:
      mouseClick(0, 1, x, y, 0);
      break;

    case Qt::RightButton:
      mouseClick(2, 1, x, y, 0);
      break;

    default:
      break;
  }
}

void
RZGUIGLWidget::wheelEvent(QWheelEvent *event)
{
  int x = static_cast<int>(event->position().x());
  int y = static_cast<int>(event->position().y());
  int delta = event->angleDelta().y();

  if (delta != 0) {
    if (delta < 0)
      mouseClick(4, 0, x, y, -delta);
    else
      mouseClick(3, 0, x, y, delta);
  }
}

void
RZGUIGLWidget::keyPressEvent(QKeyEvent *event)
{
  bool rotate = false;
  RZ::Real angle = 0;

  switch (event->key()) {
    case Qt::Key_Up:
      angle  = +RZGUIGL_KBD_ROT_DELTA;
      rotate = true;
      break;

    case Qt::Key_Down:
      angle  = -RZGUIGL_KBD_ROT_DELTA;
      rotate = true;
      break;

    default:
      break;
  }

  if (rotate) {
    m_view.roll(angle);
    m_newViewPort = true;
    update();
  }
}

// Shaders, resources, etc should be loaded here
void
RZGUIGLWidget::initializeGL()
{
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

  f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  for (auto p : m_model->elementList())
    p->enterOpenGL();
}

void
RZGUIGLWidget::configureViewPort()
{
  m_view.setScreenGeom(m_width, m_height);
  m_view.configureViewPort(m_width, m_height);
  
  if (m_zoomToBoxPending)
    zoomToBox(m_lastP1, m_lastP2);
  
  m_newViewPort = false;
}

void
RZGUIGLWidget::resizeGL(int w, int h)
{
  m_width  = w;
  m_height = h;

  configureViewPort();
}

void
RZGUIGLWidget::drawAxes()
{
  GLfloat aspect;
  GLfloat axisHeight = m_glAxes.height();
  
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
    glLoadIdentity();

    aspect = static_cast<GLfloat>(m_width) / static_cast<GLfloat>(m_height);
    glOrtho(-2, 2, -2 / aspect, 2 / aspect, -20, 20);

    glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);
      glBegin(GL_QUADS);

      glColor3fv(m_bgBelow);
      glVertex3f(-2, -2. / aspect, 10);
      glVertex3f(+2, -2. / aspect, 10);

      glColor3fv(m_bgAbove);
      glVertex3f(+2., 2. / aspect, 10);
      glVertex3f(-2., 2. / aspect, 10);
      glEnd();

      glEnable(GL_LIGHTING);
      glEnable(GL_DEPTH_TEST);

      glTranslatef(2 - 1.5 * axisHeight, -2 / aspect + 1.5 * axisHeight, 0.);

      m_view.configureOrientation(false);
      
      m_glAxes.display();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode (GL_MODELVIEW);
}


RZ::GLCurrentView *
RZGUIGLWidget::view()
{
  return &m_view;
}

void
RZGUIGLWidget::display()
{
  glGetFloatv(GL_MODELVIEW_MATRIX, m_refMatrix);

  if (m_model != nullptr)
    displayModel(m_model);
}

void
RZGUIGLWidget::paintGL()
{
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

  if (m_newViewPort)
    configureViewPort();

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();

  f->glEnable(GL_AUTO_NORMAL);
  f->glEnable(GL_NORMALIZE);

  f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (!m_fixedLight)
    configureLighting();

  drawAxes();

  if (m_fixedLight)
    configureLighting();

  m_view.configureOrientation();

  display();
}

void
RZGUIGLWidget::setModel(RZ::OMModel *model)
{
  m_model = model;
  m_selectedPath = nullptr;
  m_selectedRefFrame = nullptr;

  if (m_model != nullptr)
    m_model->recalculate();

  update();
}

//
// Relationship between x, y and a reference frame:
//   screenToWorld(wX, wY, x, y);
//       RZ::Vec3 coords(wX, wY, 0);
//       RZ::Vec3 screenNormal, screenCoords;
//       RZ::IncrementalRotation correctedRotation = m_view.rotation;
//       correctedRotation.rotateRelative(RZ::Vec3::eX(), -M_PI / 2);
//       correctedRotation.rotateRelative(RZ::Vec3::eZ(), -M_PI / 2);


void
RZGUIGLWidget::zoomToBox(RZ::Vec3 const &p1, RZ::Vec3 const &p2)
{
  m_lastP1 = p1;
  m_lastP2 = p2;

  if (m_width == 0 || m_height == 0 || m_model == nullptr) {
    m_zoomToBoxPending = true;
  } else {
    m_view.zoomToBox(*m_model->world(), m_lastP1, m_lastP2);
    display();
    m_zoomToBoxPending = false;
  }
}

void
RZGUIGLWidget::getCurrentRot(GLfloat *rot) const
{
  memcpy(rot, m_curAzEl, sizeof(GLfloat) * 3);
}

void
RZGUIGLWidget::setCurrentRot(const GLfloat *rot)
{
  if (!RZ::isZero(rot[0]))
    m_view.setRotation(RZ::Vec3::eY(), rot[0]);      
  else if (!RZ::isZero(rot[2]))
    m_view.rotateRelative(RZ::Vec3::eZ(), rot[2]);
  else
    m_view.setRotation(RZ::Vec3::eX(), rot[1]);

  update();
}

void
RZGUIGLWidget::rotateToCurrentFrame()
{
  if (m_selectedRefFrame != nullptr) {
    RZ::Vec3 center = m_selectedRefFrame->getCenter();
    
    m_view.setRotation(m_selectedRefFrame->getOrientation().t());

    RZ::Vec3 result = m_view.rotation.matrix() * center;
    
    m_view.rotateRelative(RZ::Vec3::eZ(), 90);
    m_view.rotateRelative(RZ::Vec3::eX(), 90);
    
    auto cX = -.25 * result.x * m_view.zoomLevel * m_width;
    auto cY = +.25 * result.y * m_view.zoomLevel * m_width;
    
    m_view.setCenter(cX, cY);

    m_newViewPort = true;
    update();
  }
}
