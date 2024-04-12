#include <GLUTEngine.h>
#include <GLModel.h>
#include <stdexcept>

using namespace RZ;

#define GLUT_ENGINE_SHIFT_DELTA 2e-1

GLUTEngine *GLUTEngine::currentInstance = nullptr;

void
GLUTEngine::staticMouseClick(int button, int state, int x, int y)
{
  GLUTEngine::instance()->mouseClick(button, state, x, y);
}

void
GLUTEngine::staticMouseMotion(int x, int y)
{
  GLUTEngine::instance()->mouseMotion(x, y);
}

void
GLUTEngine::staticDisplayFunc()
{
  GLUTEngine::instance()->showScreen();
}

void
GLUTEngine::staticKeyboardFunc(int c, int x, int y)
{
  GLUTEngine::instance()->keyPress(c, x, y);
}

void
GLUTEngine::adjustViewPort()
{
  GLfloat aspect;

  m_width  = glutGet(GLUT_WINDOW_WIDTH);
  m_height = glutGet(GLUT_WINDOW_HEIGHT);
  
  // Phase 1: Set viewport
  glViewport(0, 0, m_width, m_height);

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

  // Phase 3: Configure normals
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_AUTO_NORMAL);
  glEnable(GL_NORMALIZE);
}

void
GLUTEngine::showScreen()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  adjustViewPort();

  glClearColor(0, 0, .4, 1);
  
  if (model() != nullptr) {
    if (!m_fixedLight)
      model()->configureLighting();

      glTranslatef(0, 0, -10.);
      auto k = m_incRot.k();
      auto theta = RZ::rad2deg(m_incRot.theta());

      glRotatef(theta, k.x, k.y, k.z);

      glRotatef(-90, 1, 0, 0);
      glRotatef(-90, 0, 0, 1);

    if (m_fixedLight)
      model()->configureLighting();
    
    model()->display();
  }

  glutSwapBuffers();
}

void
GLUTEngine::keyPress(int c, int x, int y)
{
  RZ::Real angle = 0;
  bool rotate = false;

  switch (c) {
    case GLUT_KEY_LEFT:
      angle = RZ::deg2rad(RZ_GLUT_ENGINE_KBD_ROT_DELTA);
      rotate = true;
      break;

    case GLUT_KEY_RIGHT:
      angle = -RZ::deg2rad(RZ_GLUT_ENGINE_KBD_ROT_DELTA);
      rotate = true;
      break;
  }

    if (rotate) {
      m_incRot.rotate(RZ::Vec3::eZ(), angle);
      m_newViewPort = true;
    }
}

void
GLUTEngine::mouseClick(int button, int state, int x, int y)
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
        m_zoom *= 1.1;
        newZoom = true;
        break;

      case 4: // Mouse wheel down
        m_zoom /= 1.1;
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
}

void
GLUTEngine::mouseMotion(int x, int y)
{
  GLfloat shiftX, shiftY;

  if (m_dragging) {
    shiftX = x - m_dragStart[0];
    shiftY = y - m_dragStart[1];
    m_currentCenter[0] = m_oldCenterCenter[0] + shiftX;
    m_currentCenter[1] = m_oldCenterCenter[1] + shiftY;

    m_newViewPort = true;
  }

  if (m_rotating) {
    shiftX = x - m_rotStart[0];
    shiftY = y - m_rotStart[1];
    m_curAzEl[0] = m_oldRot[0] + shiftX * RZ_GLUT_ENGINE_MOUSE_ROT_DELTA;
    m_curAzEl[1] = m_oldRot[1] + shiftY * RZ_GLUT_ENGINE_MOUSE_ROT_DELTA;

    Real deltaX = x - m_prevRotX;
    Real deltaY = y - m_prevRotY;

    m_incRot.rotate(
      Vec3::eY(),
      deg2rad(deltaX * RZ_GLUT_ENGINE_MOUSE_ROT_DELTA));

    m_incRot.rotate(
      Vec3::eX(),
      deg2rad(deltaY * RZ_GLUT_ENGINE_MOUSE_ROT_DELTA));

    m_prevRotX = x;
    m_prevRotY = y;

    m_newViewPort = true;
  }
}

GLUTEngine::GLUTEngine()
{
  // Use a single buffered window in RGB mode (as opposed to a double-buffered
  // window or color-index mode).
  int argc = 1;
  char procName[] = "GLUTEngine";
  char *argv[] = {procName, nullptr};

  currentInstance = this;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(1024, 768);

  // Position window at (80,80)-(480,380) and give it a title.
  glutInitWindowPosition(80, 80);
  
  m_hWnd = glutCreateWindow("RayZaler Model Viewer");

  // Use this method to forward events to the right class
  glutDisplayFunc(GLUTEngine::staticDisplayFunc);
  glutIdleFunc(GLUTEngine::staticDisplayFunc);
  glutMouseFunc(GLUTEngine::staticMouseClick);
  glutMotionFunc(GLUTEngine::staticMouseMotion);
  glutSpecialFunc(GLUTEngine::staticKeyboardFunc);
}

GLUTEngine::~GLUTEngine()
{
  currentInstance = nullptr;
}

GLUTEngine *
GLUTEngine::instance()
{
  if (currentInstance == nullptr)
    currentInstance = new GLUTEngine;

  return currentInstance;
}

void
GLUTEngine::start()
{
  glutMainLoop();
}

void
GLUTEngine::setFixedLight(bool value)
{
  m_fixedLight = value;
}
