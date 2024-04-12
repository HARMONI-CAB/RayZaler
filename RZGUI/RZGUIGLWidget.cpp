#include "RZGUIGLWidget.h"
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QWheelEvent>
#include <GLHelpers.h>
#include <GL/glut.h>
#include <QPainter>
#include "GUIHelpers.h"
#include <GenericAperture.h>
#include <RayBeamElement.h>
#include <QKeyEvent>

#define RZGUIGL_MOUSE_ROT_DELTA 2e-1
#define RZGUIGL_KBD_ROT_DELTA   5

RZGUIGLWidget::RZGUIGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
  m_axisCylinder.setHeight(1e-1);
  m_axisCylinder.setRadius(5e-3);
  m_axisCylinder.setSlices(24);
  m_axisCylinder.setVisibleCaps(true, false);

  m_axisArrow.setHeight(m_axisCylinder.height() / 3);
  m_axisArrow.setBase(2 * m_axisCylinder.radius());
  m_axisArrow.setSlices(20);
  m_axisArrow.setStacks(20);

  setMouseTracking(true);
}

void
RZGUIGLWidget::pushReferenceFrameMatrix(const RZ::ReferenceFrame *frame)
{
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
      RZ::GenericAperture *ap = surf->processor->aperture();

      if (ap != nullptr) {
        pushReferenceFrameMatrix(surf->frame);
        if (m_displayElements)
          glTranslatef(0, 0, 1e-3);
        ap->renderOpenGL();
        glPopMatrix();
      }
    }
    glPopAttrib();
  }
}

void
RZGUIGLWidget::displayModel(RZ::OMModel *model)
{
  RZ::RayBeamElement *beam = static_cast<RZ::RayBeamElement *>(model->beam());

  beam->setDynamicAlpha(m_displayElements);

  if (m_displayElements) {
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

  pushElementMatrix(beam);
  beam->renderOpenGL();
  popElementMatrix();

  if (m_displayElements) {
    for (auto p : model->elementList()) {
      if (p == beam)
        continue;
      pushElementMatrix(p);
      if (m_displayNames)
        renderText(0, 0, 0, QString::fromStdString(p->name()));
      p->renderOpenGL();
      popElementMatrix();

      if (m_displayApertures)
        displayApertures(p);

      if (p->nestedModel() != nullptr)
        displayModel(p->nestedModel());
    }
  } else {
    for (auto p : model->elementList()) {
      if (p == beam)
        continue;

      if (m_displayNames) {
        pushElementMatrix(p);
        renderText(0, 0, 0, QString::fromStdString(p->name()));
        popElementMatrix();
      }

      if (m_displayApertures)
        displayApertures(p);

      if (p->nestedModel() != nullptr)
        displayModel(p->nestedModel());
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
        m_rotStart[0] = m_prevRotX = x;
        m_rotStart[1] = m_prevRotY = y;
        m_oldRot[0] = m_curAzEl[0];
        m_oldRot[1] = m_curAzEl[1];
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
    m_curAzEl[0] = m_oldRot[0] + shiftX * RZGUIGL_MOUSE_ROT_DELTA;
    m_curAzEl[1] = m_oldRot[1] + shiftY * RZGUIGL_MOUSE_ROT_DELTA;

    RZ::Real deltaX = x - m_prevRotX;
    RZ::Real deltaY = y - m_prevRotY;

    m_incRot.rotate(
      RZ::Vec3::eY(),
      RZ::deg2rad(deltaX * RZGUIGL_MOUSE_ROT_DELTA));

    m_incRot.rotate(
      RZ::Vec3::eX(),
      RZ::deg2rad(deltaY * RZGUIGL_MOUSE_ROT_DELTA));

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
      angle = RZ::deg2rad(RZGUIGL_KBD_ROT_DELTA);
      rotate = true;
      break;

    case Qt::Key_Down:
      angle = -RZ::deg2rad(RZGUIGL_KBD_ROT_DELTA);
      rotate = true;
      break;

    default:
      break;
  }

  if (rotate) {
    m_incRot.rotate(RZ::Vec3::eZ(), angle);
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
  glOrtho(-2, 2, -2 / aspect, 2 / aspect, -1000 * m_zoom, 1000 * m_zoom);
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

#define ABOVE_RED   1
#define ABOVE_GREEN 1
#define ABOVE_BLUE  1

#define BELOW_RED   0x75
#define BELOW_GREEN 0x75
#define BELOW_BLUE  0xe9
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
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);
      glBegin(GL_QUADS);

      glColor3f(BELOW_RED / 255., BELOW_GREEN / 255., BELOW_BLUE / 255.);
      glVertex3f(-2, -2. / aspect, 10);
      glVertex3f(+2, -2. / aspect, 10);

      glColor3f(ABOVE_RED / 255., ABOVE_GREEN / 255., ABOVE_BLUE / 255.);
      glVertex3f(+2., 2. / aspect, 10);
      glVertex3f(-2., 2. / aspect, 10);
      glEnd();
      glEnable(GL_LIGHTING);
      glEnable(GL_DEPTH_TEST);

      glTranslatef(2 - 1.5 * axisHeight, -2 / aspect + 1.5 * axisHeight, 0.);

      auto k = m_incRot.k();
      auto theta = RZ::rad2deg(m_incRot.theta());

      glRotatef(theta, k.x, k.y, k.z);

      glRotatef(-90, 1, 0, 0);
      glRotatef(-90, 0, 0, 1);

      glMaterialfv(GL_FRONT, GL_AMBIENT,  vec.get(0.0, 0.0, 0.0));

      // Z-axis
      glPushMatrix();
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(0, 0, 1.));
        glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, 0, .1));
        m_axisCylinder.display();
        glTranslatef(0, 0, axisHeight);
        m_axisArrow.display();
      glPopMatrix();

      // Y-axis
      glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(0, 1, 0));
        glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, .1, 0));
        m_axisCylinder.display();
        glTranslatef(0, 0, axisHeight);
        m_axisArrow.display();
      glPopMatrix();

      // X-axis
      glPushMatrix();
        glRotatef(+90, 0, 1, 0);

        glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(1, 0, 0));
        glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(.1, 0, 0));
        m_axisCylinder.display();
        glTranslatef(0, 0, axisHeight);
        m_axisArrow.display();
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
  auto k = m_incRot.k();
  auto theta = RZ::rad2deg(m_incRot.theta());

  glRotatef(theta, k.x, k.y, k.z);

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
    RZ::GLCube(1);
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
  memcpy(rot, m_curAzEl, sizeof(GLfloat) * 3);
}

void
RZGUIGLWidget::setCurrentRot(const GLfloat *rot)
{
  if (!RZ::isZero(rot[0]))
    m_incRot.setRotation(RZ::Vec3::eY(), RZ::deg2rad(rot[0]));      
  else if (!RZ::isZero(rot[2]))
    m_incRot.rotateRelative(RZ::Vec3::eZ(), RZ::deg2rad(rot[2]));
  else
    m_incRot.setRotation(RZ::Vec3::eX(), RZ::deg2rad(rot[1]));

  update();
}
