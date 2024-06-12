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
  view()->configureViewPort(m_width, m_height);
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

    view()->configureOrientation();

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
      angle  = +RZ_GLUT_ENGINE_KBD_ROT_DELTA;
      rotate = true;
      break;

    case GLUT_KEY_RIGHT:
      angle  = -RZ_GLUT_ENGINE_KBD_ROT_DELTA;
      rotate = true;
      break;
  }

  if (rotate) {
    view()->roll(angle);
    m_newViewPort = true;
  }
}

void
GLUTEngine::mouseClick(int button, int state, int x, int y)
{
  Real wX, wY;
  
  view()->screenToWorld(wX, wY, x, y);

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
        m_oldCenterCenter[0] = view()->center[0];
        m_oldCenterCenter[1] = view()->center[1];

        break;

      case 2: // Right button
        m_rotating = true;
        m_rotStart[0] = m_prevRotX = x;
        m_rotStart[1] = m_prevRotY = y;
        m_oldRot[0] = m_curAzEl[0];
        m_oldRot[1] = m_curAzEl[1];
        break;

      case 3: // Mouse wheel up
        view()->zoom(1.1);
        newZoom = true;
        break;

      case 4: // Mouse wheel down
        view()->zoom(1 / 1.1);

        newZoom = true;
        break;
    }

    if (newZoom) {
      Real prevSX, prevSY;
      Real dX, dY;

      view()->worldToScreen(prevSX, prevSY, wX, wY);

      dX = x - prevSX;
      dY = y - prevSY;

      view()->move(-dX, -dY);
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
    view()->setCenter(m_oldCenterCenter[0] + shiftX, m_oldCenterCenter[1] + shiftY);

    m_newViewPort = true;
  }

  if (m_rotating) {
    shiftX = x - m_rotStart[0];
    shiftY = y - m_rotStart[1];
    m_curAzEl[0] = m_oldRot[0] + shiftX * RZ_GLUT_ENGINE_MOUSE_ROT_DELTA;
    m_curAzEl[1] = m_oldRot[1] + shiftY * RZ_GLUT_ENGINE_MOUSE_ROT_DELTA;

    Real deltaX = x - m_prevRotX;
    Real deltaY = y - m_prevRotY;

    view()->incAzEl(
      deltaX * RZ_GLUT_ENGINE_MOUSE_ROT_DELTA,
      deltaY * RZ_GLUT_ENGINE_MOUSE_ROT_DELTA);

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

  m_width  = 1024;
  m_height = 768;

  view()->setScreenGeom(m_width, m_height);
  glutInitWindowSize(m_width, m_height);

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
