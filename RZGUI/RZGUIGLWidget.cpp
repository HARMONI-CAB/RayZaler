#include "RZGUIGLWidget.h"
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QWheelEvent>
#include <GLHelpers.h>
#include <GL/glut.h>

#define GLUT_ENGINE_SHIFT_DELTA 2e-1

RZGUIGLWidget::RZGUIGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
  m_axisCylinder.setHeight(1e-1);
  m_axisCylinder.setRadius(5e-3);
  m_axisCylinder.setSlices(24);
  m_axisCylinder.setVisibleCaps(true, false);

  setMouseTracking(true);
}

void
RZGUIGLWidget::pushElementMatrix(RZ::Element *element)
{
  auto frame = element->parentFrame();
  auto R     = frame->getOrientation();
  auto O     = frame->getCenter();

  GLdouble viewMatrix[16] = {
    R.rows[0].coords[0], R.rows[1].coords[0], R.rows[2].coords[0], O.coords[0],
    R.rows[0].coords[1], R.rows[1].coords[1], R.rows[2].coords[1], O.coords[1],
    R.rows[0].coords[2], R.rows[1].coords[2], R.rows[2].coords[2], O.coords[2],
                      0,                   0,                   0,           1};

  glPushMatrix();
  glLoadMatrixf(m_refMatrix);
  glMultTransposeMatrixd(viewMatrix);
}

void
RZGUIGLWidget::popElementMatrix()
{
  glPopMatrix();
}

void
RZGUIGLWidget::displayModel(RZ::OMModel *model)
{
  auto beam = model->beam();

  for (auto p : model->elementList()) {
    if (p == beam)
      continue;
    pushElementMatrix(p);
    p->renderOpenGL();
    popElementMatrix();

    if (p->nestedModel() != nullptr)
      displayModel(p->nestedModel());
  }

  pushElementMatrix(beam);
  beam->renderOpenGL();
  popElementMatrix();

  for (auto p : model->elementList()) {
    if (p == beam)
      continue;
    pushElementMatrix(p);
    p->renderOpenGL();
    popElementMatrix();

    if (p->nestedModel() != nullptr)
      displayModel(p->nestedModel());
  }
}

void
RZGUIGLWidget::configureLighting()
{
  RZ::GLVectorStorage vec;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glClearColor(0, 0, .4, 1);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glLightfv(GL_LIGHT0, GL_AMBIENT,  vec.get(1.0, 1.0, 1.0));
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  vec.get(1.0, 1.0, 1.0));
  glLightfv(GL_LIGHT0, GL_SPECULAR, vec.get(1.0, 1.0, 1.0));
  glLightfv(GL_LIGHT0, GL_POSITION, vec.get(1.0, 5.0, 5.0));
  glEnable(GL_LIGHT0);

  glLightfv(GL_LIGHT1, GL_AMBIENT,  vec.get(0.1, 0.1, 0.1));
  glLightfv(GL_LIGHT1, GL_DIFFUSE,  vec.get(.5, .5, .5));
  glLightfv(GL_LIGHT1, GL_SPECULAR, vec.get(.5, .5, .5));
  glLightfv(GL_LIGHT1, GL_POSITION, vec.get(1.0, 1.0, 50.0));
  glEnable(GL_LIGHT1);

  glShadeModel(GL_SMOOTH);
  glCullFace(GL_BACK);
}

void
RZGUIGLWidget::mouseClick(int button, int state, int x, int y, int shift)
{
  GLfloat wX, wY;
  GLfloat sX, sY;

  wX = +4 * (x - m_currentCenter[0] - m_width  * .5) / (m_zoom * m_width) - 2;
  wY = -4 * (y - m_currentCenter[1] - m_height * .5) / (m_zoom * m_width) + 2;

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
        m_oldCenterCenter[0] = m_currentCenter[0];
        m_oldCenterCenter[1] = m_currentCenter[1];

        break;

      case 2: // Right button
        m_rotating = true;
        m_rotStart[0] = x;
        m_rotStart[1] = y;
        m_oldRot[0] = m_curRot[0];
        m_oldRot[1] = m_curRot[1];
        break;

      case 3: // Mouse wheel up
        m_zoom *= pow(1.1, shift / 120.);
        newZoom = true;
        break;

      case 4: // Mouse wheel down
        m_zoom /= pow(1.1, shift / 120.);
        newZoom = true;
        break;
    }

    if (newZoom) {
      sX = x - (wX + 2) / 4 * (m_zoom * m_width) - m_width * .5;
      sY = y + (wY - 2) / 4 * (m_zoom * m_width) - m_height * .5;

      m_currentCenter[0] = sX;
      m_currentCenter[1] = sY;
    }
  }

  m_newViewPort = true;
  update();
}

void
RZGUIGLWidget::mouseMotion(int x, int y)
{
  GLfloat shiftX, shiftY;

  if (m_dragging) {
    shiftX = x - m_dragStart[0];
    shiftY = y - m_dragStart[1];
    m_currentCenter[0] = m_oldCenterCenter[0] + shiftX;
    m_currentCenter[1] = m_oldCenterCenter[1] + shiftY;

    m_newViewPort = true;
    update();
  }

  if (m_rotating) {
    shiftX = x - m_rotStart[0];
    shiftY = y - m_rotStart[1];
    m_curRot[0] = m_oldRot[0] + shiftX * GLUT_ENGINE_SHIFT_DELTA;
    m_curRot[1] = m_oldRot[1] + shiftY * GLUT_ENGINE_SHIFT_DELTA;

    m_newViewPort = true;
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

// Shaders, resources, etc should be loaded here
void
RZGUIGLWidget::initializeGL()
{
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

  f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void
RZGUIGLWidget::configureViewPort()
{
  GLfloat aspect;
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

  // Phase 1: Set viewport
  f->glViewport(0, 0, m_width, m_height);

  // Phase 2: Configure projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glScalef(m_zoom, m_zoom, m_zoom);
  glTranslatef(
    +2 * m_currentCenter[0] / (m_zoom * m_width),
    -2 * m_currentCenter[1] / (m_zoom * m_height),
    0);

  aspect = static_cast<GLfloat>(m_width) / static_cast<GLfloat>(m_height);
  glOrtho(-2, 2, -2 / aspect, 2 / aspect, -20 * m_zoom, 20 * m_zoom);
  f->glGetFloatv(GL_MODELVIEW_MATRIX, m_viewPortMatrix);

  // Phase 3: Configure normals
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();

  f->glEnable(GL_AUTO_NORMAL);
  f->glEnable(GL_NORMALIZE);

  m_newViewPort = false;
}

void
RZGUIGLWidget::resizeGL(int w, int h)
{
  m_width = w;
  m_height = h;

  configureViewPort();
}

void
RZGUIGLWidget::drawAxes()
{
  RZ::GLVectorStorage vec;
  GLfloat aspect, axisHeight = m_axisCylinder.height();
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
    glLoadIdentity();

    aspect = static_cast<GLfloat>(m_width) / static_cast<GLfloat>(m_height);
    glOrtho(-2, 2, -2 / aspect, 2 / aspect, -20, 20);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
      glTranslatef(2 - 1.5 * axisHeight, -2 / aspect + 1.5 * axisHeight, 0.);
      glRotatef(m_curRot[0], 0, 1, 0);
      glRotatef(m_curRot[1], 1, 0, 0);
      glRotatef(m_curRot[2], 0, 0, 1);

      glRotatef(-90, 1, 0, 0);
      glRotatef(-90, 0, 0, 1);

      glMaterialfv(GL_FRONT, GL_AMBIENT,  vec.get(0.0, 0.0, 0.0));

      // Z-axis
      glPushMatrix();
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(0, 0, 1.));
        glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, 0, .1));
        m_axisCylinder.display();
        glTranslatef(0, 0, axisHeight);
        glutSolidCone(2 * m_axisCylinder.radius(), axisHeight / 3, 20, 20);
      glPopMatrix();

      // Y-axis
      glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(0, 1, 0));
        glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, .1, 0));
        m_axisCylinder.display();
        glTranslatef(0, 0, axisHeight);
        glutSolidCone(2 * m_axisCylinder.radius(), axisHeight / 3, 20, 20);
      glPopMatrix();

      // X-axis
      glPushMatrix();
        glRotatef(+90, 0, 1, 0);

        glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(1, 0, 0));
        glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(.1, 0, 0));
        m_axisCylinder.display();
        glTranslatef(0, 0, axisHeight);
        glutSolidCone(2 * m_axisCylinder.radius(), axisHeight / 3, 20, 20);
      glPopMatrix();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode (GL_MODELVIEW);
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

  glTranslatef(0, 0, -10.);
  glRotatef(m_curRot[0], 0, 1, 0);
  glRotatef(m_curRot[1], 1, 0, 0);
  glRotatef(m_curRot[2], 0, 0, 1);

  glRotatef(-90, 1, 0, 0);
  glRotatef(-90, 0, 0, 1);

  f->glGetFloatv(GL_MODELVIEW_MATRIX, m_refMatrix);

  if (m_fixedLight)
    configureLighting();

  if (m_model != nullptr) {
    displayModel(m_model);
  } else {
    glRotatef(45, 1, 0, 0);
    glRotatef(45, 0, 1, 0);
    glutSolidCube(1);
  }
}

void
RZGUIGLWidget::setModel(RZ::OMModel *model)
{
  m_model = model;
  if (m_model != nullptr)
    m_model->recalculate();
}

void
RZGUIGLWidget::getCurrentRot(GLfloat *rot) const
{
  memcpy(rot, m_curRot, sizeof(GLfloat) * 3);
}

void
RZGUIGLWidget::setCurrentRot(const GLfloat *rot)
{
  memcpy(m_curRot, rot, sizeof(GLfloat) * 3);
  update();
}
