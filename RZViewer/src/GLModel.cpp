#include <GLModel.h>
#include <GLHelpers.h>

using namespace RZ;

void
GLModel::configureLighting()
{
  GLVectorStorage vec;

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
GLModel::setEventListener(GLModelEventListener *listener)
{
  m_listener = listener;
}

void
GLModel::tick()
{
  if (m_listener != nullptr)
    m_listener->tick();
}
