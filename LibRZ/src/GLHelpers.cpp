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

//////////////////////////// GLSphericalCap //////////////////////////////////
GLSphericalCap::GLSphericalCap()
{
  m_quadric = gluNewQuadric();
}

GLSphericalCap::~GLSphericalCap()
{
  gluDeleteQuadric(m_quadric);
}

//
// Inspired in http://www.songho.ca/opengl/gl_sphere.html
//
void
GLSphericalCap::recalculate()
{
  GLint i, j, n = 0, total;
  GLfloat alpha    = acos((m_radius - m_height) / m_radius);
  GLfloat rInv   = 1 / m_radius;
  GLfloat lonDelta = 2 * M_PI / m_sectors;
  GLfloat latDelta = alpha / m_stacks;
  GLfloat secDelta = 1.f / m_sectors;
  GLfloat stDelta  = 1.f / m_stacks;
  GLint k1, k2;

  total = (m_stacks + 1) * (m_sectors + 1);
  m_vertices.resize(3 * total);
  m_normals.resize(3 * total);
  m_texCoords.resize(2 * total);

  if (m_invertNormals)
    rInv = -rInv;

  for (j = 0; j <= m_stacks; ++j) {
    GLfloat lat = M_PI / 2 - j * latDelta;
    GLfloat xy  = m_radius * cosf(lat);
    GLfloat z   = m_radius * sinf(lat);

    for (i = 0; i <= m_sectors; ++i) {
      GLfloat lon = i * lonDelta;

      GLfloat x = xy * cosf(lon);
      GLfloat y = xy * sinf(lon);

      m_vertices[3 * n + 0] = x;
      m_vertices[3 * n + 1] = y;
      m_vertices[3 * n + 2] = z;

      m_normals[3 * n + 0] = rInv * x;
      m_normals[3 * n + 1] = rInv * y;
      m_normals[3 * n + 2] = rInv * z;

      m_texCoords[2 * n + 0] = i * secDelta;
      m_texCoords[2 * n + 1] = j * stDelta;

      ++n;
    }
  }

  // Calculate indices
  total = 2 * m_sectors * m_stacks - m_sectors;
  n     = 0;
  m_indices.resize(6 * total);
  int d1, d2;

  d1 = m_invertNormals ? 2 : 1;
  d2 = m_invertNormals ? 1 : 2;

  for(i = 0; i < m_stacks; ++i) {
    k1 = i * (m_sectors + 1);     // beginning of current stack
    k2 = k1 + m_sectors + 1;      // beginning of next stack

    for(j = 0; j < m_sectors; ++j, ++k1, ++k2) {
      // 2 triangles per sector excluding first and last stacks
      // k1 => k2 => k1+1
      if (i != 0) {
        m_indices[3 * n +  0] = k1;
        
        m_indices[3 * n + d1] = k2;
        m_indices[3 * n + d2] = k1 + 1;
        ++n;
      }

      // k1+1 => k2 => k2+1
      m_indices[3 * n +  0] = k1 + 1;
      m_indices[3 * n + d1] = k2;
      m_indices[3 * n + d2] = k2 + 1;
      ++n;
    }
  }

  m_dirty = false;
}

void
GLSphericalCap::setHeight(GLdouble height)
{
  m_height = height;
  m_dirty  = true;
}

void
GLSphericalCap::setRadius(GLdouble radius)
{
  m_radius = radius;
  m_dirty  = true;
}

void
GLSphericalCap::setSectors(GLint slices)
{
  m_sectors = slices;
  m_dirty   = true;
}

void
GLSphericalCap::setStacks(GLint stacks)
{
  m_stacks = stacks;
  m_dirty  = true;
}

void
GLSphericalCap::setInvertNormals(bool inv)
{
  m_invertNormals = inv;
  m_dirty         = true;
}

void
GLSphericalCap::display()
{
  if (m_dirty)
    recalculate();

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(3, GL_FLOAT,   3 * sizeof(GLfloat), m_vertices.data());
  glNormalPointer(GL_FLOAT,      3 * sizeof(GLfloat), m_normals.data());
  glTexCoordPointer(2, GL_FLOAT, 2 * sizeof(GLfloat), m_texCoords.data());

  glDrawElements(
    GL_TRIANGLES,
    (unsigned int) m_indices.size(),
    GL_UNSIGNED_INT,
    m_indices.data());

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

//////////////////////////////// GLDisc ////////////////////////////////////////
GLDisc::GLDisc()
{
  m_quadric = gluNewQuadric();
}

GLDisc::~GLDisc()
{
  gluDeleteQuadric(m_quadric);
}

void
GLDisc::recalculate()
{
  GLint i;
  GLdouble angDelta = 2 * M_PI / m_slices;
  // Generate vertices for the fans

  m_vertices.resize(3 * (m_slices + 2));

  m_vertices[0] = 0;
  m_vertices[1] = 0;
  m_vertices[2] = 0;

  for (i = 0; i < (m_slices + 1); ++i) {
    m_vertices[3 * (i + 1) + 0] = .5 * m_width  * cos(angDelta * i);
    m_vertices[3 * (i + 1) + 1] = .5 * m_height * sin(angDelta * i);
    m_vertices[3 * (i + 1) + 2] = 0;
  }

  m_dirty = false;
}

void
GLDisc::setRadius(GLdouble radius)
{
  m_width = m_height = radius;
  m_dirty  = true;
}

void
GLDisc::setHeight(GLdouble height)
{
  m_height = height;
  m_dirty  = true;
}

void
GLDisc::setWidth(GLdouble width)
{
  m_width = width;
  m_dirty = true;
}

void
GLDisc::setSlices(GLint slices)
{
  m_slices = slices;
  m_dirty  = true;
}

void
GLDisc::display()
{
  int i;

  if (m_dirty)
    recalculate();

  for (i = 0; i < 4; ++i) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0,  m_vertices.data());
    glDrawArrays(GL_TRIANGLE_FAN, 0, m_vertices.size() / 3);
    glDisableClientState(GL_VERTEX_ARRAY);

    glRotatef(90, 0, 0, 1);
  }
}


///////////////////////////////// GLPinHole ////////////////////////////////////
GLPinHole::GLPinHole()
{
  m_quadric = gluNewQuadric();
}

GLPinHole::~GLPinHole()
{
  gluDeleteQuadric(m_quadric);
}

void
GLPinHole::recalculate()
{
  GLint i;
  GLdouble angDelta = .5 * M_PI / m_slices;
  // Generate vertices for the fans

  m_vertices.resize(3 * (m_slices + 3));

  m_vertices[0] = m_width / 2;
  m_vertices[1] = m_height / 2;
  m_vertices[2] = 0;

  for (i = 0; i < (m_slices + 1); ++i) {
    m_vertices[3 * (i + 1) + 0] = m_radius * cos(angDelta * i);
    m_vertices[3 * (i + 1) + 1] = m_radius * sin(angDelta * i);
    m_vertices[3 * (i + 1) + 2] = 0;
  }

  m_vertices[3 * (m_slices + 2) + 0] = -m_width / 2;
  m_vertices[3 * (m_slices + 2) + 1] = m_height / 2;
  m_vertices[3 * (m_slices + 2) + 2] = 0;

  m_dirty = false;
}

void
GLPinHole::setRadius(GLdouble radius)
{
  m_radius = radius;
  m_dirty  = true;
}

void
GLPinHole::setHeight(GLdouble height)
{
  m_height = height;
  m_dirty  = true;
}

void
GLPinHole::setWidth(GLdouble width)
{
  m_width = width;
  m_dirty = true;
}

void
GLPinHole::setSlices(GLint slices)
{
  m_slices = slices;
  m_dirty  = true;
}

void
GLPinHole::display()
{
  int i;

  if (m_dirty)
    recalculate();

  for (i = 0; i < 4; ++i) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0,  m_vertices.data());
    glDrawArrays(GL_TRIANGLE_FAN, 0, m_vertices.size() / 3);
    glDisableClientState(GL_VERTEX_ARRAY);

    glRotatef(90, 0, 0, 1);
  }
}
