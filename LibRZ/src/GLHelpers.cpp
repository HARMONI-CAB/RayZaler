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
  // NO-OP
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
  m_topCap.setRadius(radius);
  m_bottomCap.setRadius(radius);
  m_dirty  = true;
}

void
GLCappedCylinder::setSlices(GLint slices)
{
  m_slices = slices;
  m_topCap.setSlices(slices);
  m_bottomCap.setSlices(slices);
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
  const int stride = 6 * sizeof(GLfloat);

  if (m_dirty)
    recalculateCaps();

  if (m_drawTop) {
    glPushMatrix();
    glTranslatef(0, 0, m_height);
    m_topCap.display();
    glPopMatrix();
  }

  if (m_drawBase) {
    glPushMatrix();
    glRotatef(180, 1, 0, 0);
    m_bottomCap.display();
    glPopMatrix();
  }
  
  gluCylinder(m_quadric, m_radius, m_radius, m_height, m_slices, 2);
}

////////////////////////////////// GLTube //////////////////////////////////////
GLTube::GLTube()
{
  m_outerQuadric = gluNewQuadric();
  m_innerQuadric = gluNewQuadric();

  gluQuadricOrientation(m_innerQuadric, GLU_INSIDE);
}

GLTube::~GLTube()
{
  gluDeleteQuadric(m_innerQuadric);
  gluDeleteQuadric(m_outerQuadric);
}

void
GLTube::recalculateCaps()
{
  // NO-OP
  m_dirty = false;
}

void
GLTube::setHeight(GLdouble height)
{
  m_height = height;
  m_dirty  = true;
}

void
GLTube::setInnerRadius(GLdouble radius)
{
  m_innerRadius = radius;
  m_topCap.setInnerRadius(radius);
  m_bottomCap.setInnerRadius(radius);
  m_dirty  = true;
}

void
GLTube::setOuterRadius(GLdouble radius)
{
  m_outerRadius = radius;
  m_topCap.setOuterRadius(radius);
  m_bottomCap.setOuterRadius(radius);
  m_dirty  = true;
}


void
GLTube::setSlices(GLint slices)
{
  m_slices = slices;
  m_topCap.setSlices(slices);
  m_bottomCap.setSlices(slices);
  m_dirty  = true;
}

void
GLTube::setVisibleCaps(bool base, bool top)
{
  m_drawBase = base;
  m_drawTop  = top;
}

void
GLTube::display()
{
  const int stride = 6 * sizeof(GLfloat);

  if (m_dirty)
    recalculateCaps();

  gluCylinder(
    m_innerQuadric,
    m_innerRadius,
    m_innerRadius,
    m_height,
    m_slices,
    2);

  gluCylinder(
    m_outerQuadric,
    m_outerRadius,
    m_outerRadius,
    m_height,
    m_slices,
    2);

  if (m_drawTop) {
    glPushMatrix();
    glTranslatef(0, 0, m_height);
    m_topCap.display();
    glPopMatrix();
  }

  if (m_drawBase) {
    glPushMatrix();
    glRotatef(180, 1, 0, 0);
    m_bottomCap.display();
    glPopMatrix();
  }
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

//////////////////////////// GLSphericalCap //////////////////////////////////
GLParabolicCap::GLParabolicCap()
{
  m_quadric = gluNewQuadric();
}

GLParabolicCap::~GLParabolicCap()
{
  gluDeleteQuadric(m_quadric);
}

//
// Inspired in http://www.songho.ca/opengl/gl_sphere.html
//
void
GLParabolicCap::recalculate()
{
  GLint i, j, n    = 0, total;
  GLfloat angDelta = 2 * M_PI / m_sectors;
  GLfloat rDelta   = m_radius / m_stacks;
  GLfloat secDelta = 1.f / m_sectors;
  GLfloat stDelta  = 1.f / m_stacks;
  GLfloat k        = .25 / m_flength;
  GLfloat R2       = m_radius * m_radius;
  GLint k1, k2;

  total = (m_stacks + 1) * (m_sectors + 1);
  m_vertices.resize(3 * total);
  m_normals.resize(3 * total);
  m_texCoords.resize(2 * total);

  // This loop traverses the radius
  for (j = 0; j <= m_stacks; ++j) {
    GLfloat r = rDelta * j;
    GLfloat nInv = 1. / sqrt(4 * k * k * r * r + 1);
    if (m_invertNormals)
      nInv = -nInv;

    // And this other one, the angles
    for (i = 0; i <= m_sectors; ++i) {
      GLfloat ang = i * angDelta;
      
      GLfloat x = r * cosf(ang);
      GLfloat y = r * sinf(ang);
      
      m_vertices[3 * n + 0] = x;
      m_vertices[3 * n + 1] = y;
      m_vertices[3 * n + 2] = k * (x * x + y * y - R2);

      m_normals[3 * n + 0] = -2 * k * x * nInv;
      m_normals[3 * n + 1] = -2 * k * y * nInv;
      m_normals[3 * n + 2] = nInv;

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
GLParabolicCap::setFocalLength(GLdouble height)
{
  m_flength = height;
  m_dirty   = true;
}

void
GLParabolicCap::setRadius(GLdouble radius)
{
  m_radius = radius;
  m_dirty  = true;
}

void
GLParabolicCap::setSectors(GLint slices)
{
  m_sectors = slices;
  m_dirty   = true;
}

void
GLParabolicCap::setStacks(GLint stacks)
{
  m_stacks = stacks;
  m_dirty  = true;
}

void
GLParabolicCap::setInvertNormals(bool inv)
{
  m_invertNormals = inv;
  m_dirty         = true;
}

void
GLParabolicCap::display()
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
  GLint i, n = 0, total;
  GLfloat sliceDelta = 1. / m_slices;
  GLfloat angDelta = 2 * M_PI * sliceDelta;
  GLfloat ang;

  total = m_slices + 2;
  m_vertices.resize(3 * total);
  m_normals.resize(3 * total);
  m_texCoords.resize(2 * total);

  ang = 0;

  m_vertices[3 * n + 0] = 0;
  m_vertices[3 * n + 1] = 0;
  m_vertices[3 * n + 2] = 0;

  m_normals[3 * n + 0] = 0;
  m_normals[3 * n + 1] = 0;
  m_normals[3 * n + 2] = 1;

  m_texCoords[2 * n + 0] = 0;
  m_texCoords[2 * n + 1] = 0;

  ++n;

  for (i = 0; i <= m_slices; ++i) {
    GLfloat x = .5 * m_width  * cosf(ang);
    GLfloat y = .5 * m_height * sinf(ang);

    m_vertices[3 * n + 0] = x;
    m_vertices[3 * n + 1] = y;
    m_vertices[3 * n + 2] = 0;

    m_normals[3 * n + 0] = 0;
    m_normals[3 * n + 1] = 0;
    m_normals[3 * n + 2] = 1;

    m_texCoords[2 * n + 0] = i * sliceDelta;
    m_texCoords[2 * n + 1] = 1;

    ang += angDelta;

    ++n;
  }

  // Calculate indices
  n     = 0;
  m_indices.resize(3 * m_slices);

  for (i = 0; i < m_slices; ++i) {
    m_indices[3 * n + 0] = 0;
    m_indices[3 * n + 1] = i + 1;
    m_indices[3 * n + 2] = i + 2;
    ++n;
  }

  m_dirty = false;
}

void
GLDisc::setRadius(GLdouble radius)
{
  m_width = m_height = 2 * radius;
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

//////////////////////////////// GLRing ////////////////////////////////////////
GLRing::GLRing()
{
  m_quadric = gluNewQuadric();
}

GLRing::~GLRing()
{
  gluDeleteQuadric(m_quadric);
}

void
GLRing::recalculate()
{
  GLint i, n = 0, total;
  GLfloat sliceDelta = 1. / m_slices;
  GLfloat angDelta = 2 * M_PI * sliceDelta;
  GLfloat ang;
  
  total = m_slices + 1;
  m_vertices.resize( 2 * 3 * total);
  m_normals.resize(  2 * 3 * total);
  m_texCoords.resize(2 * 2 * total);

  ang = 0;

  for (i = 0; i <= m_slices; ++i) {
    GLfloat x_i = m_innerRadius * cosf(ang);
    GLfloat y_i = m_innerRadius * sinf(ang);
    GLfloat x_o = m_outerRadius * cosf(ang);
    GLfloat y_o = m_outerRadius * sinf(ang);

    // Inner vertex
    m_vertices[3 * n + 0] = x_i;
    m_vertices[3 * n + 1] = y_i;
    m_vertices[3 * n + 2] = 0;

    m_normals[3 * n + 0] = 0;
    m_normals[3 * n + 1] = 0;
    m_normals[3 * n + 2] = 1;

    m_texCoords[2 * n + 0] = i * sliceDelta;
    m_texCoords[2 * n + 1] = 0;
    ++n;

    // Outer vertex
    m_vertices[3 * n + 0] = x_o;
    m_vertices[3 * n + 1] = y_o;
    m_vertices[3 * n + 2] = 0;

    m_normals[3 * n + 0] = 0;
    m_normals[3 * n + 1] = 0;
    m_normals[3 * n + 2] = 1;

    m_texCoords[2 * n + 0] = i * sliceDelta;
    m_texCoords[2 * n + 1] = 1;
    ++n;

    ang += angDelta;
  }

  // Calculate indices that well be used as triangle strip. The ordering would
  // be as follows:
  //
  // p_o[0], p_i[0], p_o[1], p_i[1], (...), p_i[n-1], p_o[0], p_i[0]
  //
  // Which is:
  //
  // v[0], v[1], v[2], v[3], (...), v[2n - 1], v[0], v[1]
  //
  n     = 0;
  m_indices.resize(2 * m_slices + 2);

  for (i = 0; i < 2 * m_slices; ++i)
    m_indices[i] = i;
  
  m_indices[2 * m_slices]     = 0;
  m_indices[2 * m_slices + 1] = 1;

  m_dirty = false;
}

void
GLRing::setInnerRadius(GLdouble radius)
{
  m_innerRadius = radius;
  m_dirty  = true;
}

void
GLRing::setOuterRadius(GLdouble radius)
{
  m_outerRadius = radius;
  m_dirty  = true;
}

void
GLRing::setSlices(GLint slices)
{
  m_slices = slices;
  m_dirty  = true;
}

void
GLRing::display()
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
    GL_TRIANGLE_STRIP,
    (unsigned int) m_indices.size(),
    GL_UNSIGNED_INT,
    m_indices.data());

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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

template <typename T> int sgn(T val) {
  return (T(0) < val) - (val < T(0));
}

void
GLPinHole::recalculate()
{
  GLint i, j, stride, p;
  GLdouble angDelta = .5 * M_PI / m_slices;
  GLdouble angle = 0;
  // Generate vertices for the fans

  stride = 3 * (m_slices + 3);
  m_vertices.resize(4 * stride);

  p = 0;
  for (j = 0; j < 4; ++j) {
    m_vertices[p++] = (m_width  / 2) * sgn(cos(angle + M_PI / 4));
    m_vertices[p++] = (m_height / 2) * sgn(sin(angle + M_PI / 4));
    m_vertices[p++] = 0;

    for (i = 0; i <= m_slices; ++i) {
      m_vertices[p++] = m_radius * cos(angle);
      m_vertices[p++] = m_radius * sin(angle);
      m_vertices[p++] = 0;

      if (i < m_slices)
        angle += angDelta;
    }

    m_vertices[p++] = (m_width   / 2) * sgn(cos(angle + M_PI / 4));
    m_vertices[p++] = (m_height  / 2) * sgn(sin(angle + M_PI / 4));
    m_vertices[p++] = 0;
  }

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
  GLint stride = 3 * (m_slices + 3);

  if (m_dirty)
    recalculate();

  for (i = 0; i < 4; ++i) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0,  m_vertices.data() + i * stride);
    glDrawArrays(GL_TRIANGLE_FAN, 0, stride / 3);
    glDisableClientState(GL_VERTEX_ARRAY);
  }
}
