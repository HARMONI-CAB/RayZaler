#define GL_GLEXT_PROTOTYPES

#include <GLHelpers.h>
#include <GL/gl.h>
#include <cmath>
#include <cstdio>

using namespace RZ;

//////////////////////////// GLCappedCylinder //////////////////////////////////
GLCappedCylinder::GLCappedCylinder()
{
  m_quadric = gluNewQuadric();
}

GLCappedCylinder::~GLCappedCylinder()
{
  gluDeleteQuadric(m_quadric);
}

void
GLCappedCylinder::recalculateCaps()
{
  GLint i;
  GLdouble angDelta = 2 * M_PI / m_slices;
  // Generate vertices for the fans

  m_topCapVertices.resize(3 * (m_slices + 2));
  m_baseCapVertices.resize(3 * (m_slices + 2));

  m_baseCapVertices[0] = 0;
  m_baseCapVertices[1] = 0;
  m_baseCapVertices[2] = 0;

  m_topCapVertices[0] = 0;
  m_topCapVertices[1] = 0;
  m_topCapVertices[2] = m_height;

  for (i = 0; i < (m_slices + 1); ++i) {
    m_baseCapVertices[3 * (i + 1) + 0] = m_baseCapVertices[0] + m_radius * cos(angDelta * i);
    m_baseCapVertices[3 * (i + 1) + 1] = m_baseCapVertices[1] + m_radius * sin(-angDelta * i);
    m_baseCapVertices[3 * (i + 1) + 2] = m_baseCapVertices[2];

    m_topCapVertices[3 * (i + 1) + 0] = m_topCapVertices[0] + m_radius * cos(angDelta * i);
    m_topCapVertices[3 * (i + 1) + 1] = m_topCapVertices[1] + m_radius * sin(angDelta * i);
    m_topCapVertices[3 * (i + 1) + 2] = m_topCapVertices[2];
  }
  m_dirty = false;
}

void
GLCappedCylinder::setHeight(GLdouble height)
{
  m_height = height;
  m_dirty  = true;
}

void
GLCappedCylinder::setRadius(GLdouble radius)
{
  m_radius = radius;
  m_dirty  = true;
}

void
GLCappedCylinder::setSlices(GLint slices)
{
  m_slices = slices;
  m_dirty  = true;
}

void
GLCappedCylinder::setVisibleCaps(bool base, bool top)
{
  m_drawBase = base;
  m_drawTop  = top;
}

void
GLCappedCylinder::display()
{
  if (m_dirty)
    recalculateCaps();

  glEnableClientState(GL_VERTEX_ARRAY);

  if (m_drawTop) {
    glVertexPointer(3, GL_FLOAT, 0,  m_topCapVertices.data());
    glDrawArrays(GL_TRIANGLE_FAN, 0, m_topCapVertices.size() / 3);
  }

  if (m_drawBase) {
    glVertexPointer(3, GL_FLOAT, 0,  m_baseCapVertices.data());
    glDrawArrays(GL_TRIANGLE_FAN, 0, m_baseCapVertices.size() / 3);
  }
  
  glDisableClientState(GL_VERTEX_ARRAY);

  gluCylinder(m_quadric, m_radius, m_radius, m_height, m_slices, 2);
}
