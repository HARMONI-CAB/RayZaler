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

#define GL_GLEXT_PROTOTYPES

#include <GLHelpers.h>
#include <GL/gl.h>
#include <cmath>
#include <cstdio>
#include <Vector.h>
#include <Singleton.h>
#include <FT2Facade.h>
#include <Logger.h>

using namespace RZ;

static inline void
normalizef(GLfloat *v)
{
  GLfloat norm = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  GLfloat k    = 1 / norm;

  v[0] *= k;
  v[1] *= k;
  v[2] *= k;
}

///////////////////////////////// GLShader /////////////////////////////////////
//
// Refactorized from https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader_s.h
//

GLShader::GLShader(const char *vertexCode, const char *fragCode)
{
  unsigned int vertex, fragment;

  // Create shader program
  m_id = glCreateProgram();

  // Allocate shaders
  if (vertexCode != nullptr) {
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexCode, nullptr);
    glCompileShader(vertex);
    if (!checkBuildErrors(vertex, "VERTEX")) {
      RZError("Failed to build vertex shader\n");
      goto done;
    }

    glAttachShader(m_id, vertex);
  }

  if (fragCode != nullptr) {
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragCode, nullptr);
    glCompileShader(fragment);
    if (!checkBuildErrors(fragment, "FRAGMENT")) {
      RZError("Failed to build fragment shader\n");
      goto done;
    }

    glAttachShader(m_id, fragment);
  }

  glLinkProgram(m_id);
  if (!checkBuildErrors(m_id, "PROGRAM")) {
    RZError("Failed to link program\n");
    goto done;
  }

  m_initialized = true;

done:
  // delete the shaders as they're linked into our program now and no longer necessary

  if (vertexCode != nullptr)
    glDeleteShader(vertex);

  if (fragCode != nullptr)
    glDeleteShader(fragment);
}

GLShader::~GLShader()
{
  if (m_id >= 0)
    glDeleteProgram(m_id);
}

void
GLShader::use()
{
  if (m_initialized)
    glUseProgram(m_id);
}

void
GLShader::leave()
{
  if (m_initialized)
    glUseProgram(0);
}

void
GLShader::set(std::string const &name, unsigned int value) const
{
  if (m_initialized)
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void
GLShader::set(std::string const &name, Vec3 const &value) const
{
  if (m_initialized) {
    glUniform3f(
      glGetUniformLocation(m_id, name.c_str()),
      value.x,
      value.y,
      value.z);
  }
}

void
GLShader::set(std::string const &name, bool value) const
{
  if (m_initialized)
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void
GLShader::set(std::string const &name, int value) const
{
  if (m_initialized)
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void
GLShader::set(std::string const &name, Real value) const
{
  if (m_initialized)
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

bool
GLShader::checkBuildErrors(unsigned int shader, std::string const &type)
{
  int success = 0;
  std::vector<char> buf;

  buf.resize(1024);

  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, buf.size(), nullptr, buf.data());
      RZError("Shader build error: %s\n", buf.data());
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, buf.size(), nullptr, buf.data());
      RZError("Shader linking error: %s\n", buf.data());
    }
  }

  return success != 0;
}

///////////////////////////////// GLCube ///////////////////////////////////////
//
// Normals and faces are numerated according to original SGI implementation
// back in 1993.
//
// See: https://github.com/markkilgard/glut/blob/master/lib/glut/glut_shapes.c
//
void
RZ::GLCube(GLfloat size, bool wireFrame)
{
  static const GLfloat normals[6][3] = {
    {-1, 0, 0},
    {0, +1, 0},
    {+1, 0, 0},
    {0, -1, 0},
    {0, 0, +1},
    {0, 0, -1}
  };

  static const GLint faces[6][4] = {
    {0, 1, 2, 3},
    {3, 2, 6, 7},
    {7, 6, 5, 4},
    {4, 5, 1, 0},
    {5, 6, 2, 1},
    {7, 4, 0, 3}
  };

  GLfloat v[8][3];

  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
  
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = +size / 2;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = +size / 2;  
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = +size / 2;

  for (auto i = 5; i >= 0; --i) {
    glBegin(wireFrame ? GL_LINE_LOOP : GL_QUADS);
    if (!wireFrame)
      glNormal3fv(normals[i]);
    glVertex3fv(v[faces[i][0]]);
    glVertex3fv(v[faces[i][1]]);
    glVertex3fv(v[faces[i][2]]);
    glVertex3fv(v[faces[i][3]]);
    glEnd();
  }
  
}


/////////////////////////////////// GLCone /////////////////////////////////////
GLCone::GLCone()
{
  m_quadric = gluNewQuadric();
  gluQuadricDrawStyle(m_quadric, GLU_FILL);
  gluQuadricNormals(m_quadric, GLU_SMOOTH);
}

GLCone::~GLCone()
{
  if (m_quadric != nullptr)
    gluDeleteQuadric(m_quadric);
}

void
GLCone::setBase(GLdouble base)
{
  m_base = base;
}

void
GLCone::setHeight(GLdouble height)
{
  m_height = height;
}

void
GLCone::setSlices(GLint slices)
{
  m_slices = slices;
}

void
GLCone::setStacks(GLint stacks)
{
  m_stacks = stacks;
}

void
GLCone::display()
{
  gluCylinder(m_quadric, m_base, 0, m_height, m_slices, m_stacks);
}

//////////////////////////// GLCappedCylinder //////////////////////////////////
GLCappedCylinder::GLCappedCylinder()
{
  m_quadric = gluNewQuadric();
  setCaps(&m_topDiscCap, &m_bottomDiscCap);
}

GLCappedCylinder::~GLCappedCylinder()
{
  if (m_quadric != nullptr)
    gluDeleteQuadric(m_quadric);
}

void
GLCappedCylinder::recalculateCaps()
{
  m_topCap->requestRecalc();
  m_bottomCap->requestRecalc();
  
  auto &edgeTop    = *m_topCap->edge();
  auto &edgeBottom = *m_bottomCap->edge();
  auto &edge1 = m_invertNormals ? edgeBottom : edgeTop;
  auto &edge2 = m_invertNormals ? edgeTop    : edgeBottom;
  GLfloat edge1height = m_invertNormals ? 0 : m_height;
  GLfloat edge2height = m_invertNormals ? m_height : 0;
  size_t rects = edgeTop.size() / 3;
  unsigned int n   = 0;
  GLfloat sign = m_invertNormals ? -1 : 1;

  if (edgeTop.size() != edgeBottom.size())
    return;

  if (edgeTop.size() < 3)
    return;

  std::vector<GLfloat> faceNormals;

  m_strip.resize(2 * 3 * (rects + 1));
  m_normals.resize(2 * 3 * (rects + 1));
  
  faceNormals.resize(3 * 2 * rects);


  // Calculate vertices
  for (auto i = 0; i < rects; ++i) {
    m_strip[3 * n + 0] = edge1[3 * i + 0];
    m_strip[3 * n + 1] = edge1[3 * i + 1];
    m_strip[3 * n + 2] = edge1[3 * i + 2] + edge1height;
    ++n;
    
    m_strip[3 * n + 0] = edge2[3 * i + 0];
    m_strip[3 * n + 1] = edge2[3 * i + 1];
    m_strip[3 * n + 2] = edge2[3 * i + 2] + edge2height;
    ++n;
  }

  m_strip[3 * n + 0] = edge1[0];
  m_strip[3 * n + 1] = edge1[1];
  m_strip[3 * n + 2] = edge1[2] + edge1height;
  ++n;
  
  m_strip[3 * n + 0] = edge2[0];
  m_strip[3 * n + 1] = edge2[1];
  m_strip[3 * n + 2] = edge2[2] + edge2height;
  ++n;

  // Calculate face normals
  const GLfloat *v = m_strip.data();

  n = 0;
  for (auto i = 0; i < rects; ++i) {
    const GLfloat *top1 = v + 3 * (2 * i + 0);
    const GLfloat *top2 = v + 3 * (2 * i + 1);
    const GLfloat *top3 = v + 3 * (2 * i + 2);

    const GLfloat *bot1 = v + 3 * (2 * i + 1);
    const GLfloat *bot3 = v + 3 * (2 * i + 2);
    const GLfloat *bot2 = v + 3 * (2 * i + 3);

    {
      const GLfloat u[3] = {top2[0] - top1[0], top2[1] - top1[1], top2[2] - top1[2]};
      const GLfloat v[3] = {top3[0] - top1[0], top3[1] - top1[1], top3[2] - top1[2]};
      
      faceNormals[3 * n + 0] = (u[1] * v[2] - u[2] * v[1]);
      faceNormals[3 * n + 1] = (u[2] * v[0] - u[0] * v[2]);
      faceNormals[3 * n + 2] = (u[0] * v[1] - u[1] * v[0]);
      normalizef(&faceNormals[3 * n]);
      ++n;
    }

    {
      const GLfloat u[3] = {bot2[0] - bot1[0], bot2[1] - bot1[1], bot2[2] - bot1[2]};
      const GLfloat v[3] = {bot3[0] - bot1[0], bot3[1] - bot1[1], bot3[2] - bot1[2]};

      faceNormals[3 * n + 0] = (u[1] * v[2] - u[2] * v[1]);
      faceNormals[3 * n + 1] = (u[2] * v[0] - u[0] * v[2]);
      faceNormals[3 * n + 2] = (u[0] * v[1] - u[1] * v[0]);
      normalizef(&faceNormals[3 * n]);
      ++n;
    }
  }

  // Interpolate normals
  n = 0;
  for (auto q = 0; q <= rects; ++q) {
    unsigned int i = (q + rects - 1) % rects;
    unsigned int j = (q) % rects;

    // Normal of the top triangle
    GLfloat anx = faceNormals[3 * 2 * i + 0];
    GLfloat any = faceNormals[3 * 2 * i + 1];
    GLfloat anz = faceNormals[3 * 2 * i + 2];
    
    // Normal of the bottom triangle
    GLfloat bnx = faceNormals[3 * 2 * j + 0];
    GLfloat bny = faceNormals[3 * 2 * j + 1];
    GLfloat bnz = faceNormals[3 * 2 * j + 2];

    m_normals[3 * n + 0] = .5 * (anx + bnx);
    m_normals[3 * n + 1] = .5 * (any + bny);
    m_normals[3 * n + 2] = .5 * (anz + bnz);
    normalizef(&m_normals[3 * n]);
    ++n;

    m_normals[3 * n + 0] = .5 * (anx + bnx);
    m_normals[3 * n + 1] = .5 * (any + bny);
    m_normals[3 * n + 2] = .5 * (anz + bnz);
    normalizef(&m_normals[3 * n]);
    ++n;
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
GLCappedCylinder::setCaps(
  GLAbstractCap *top,
  GLAbstractCap *bottom)
{
  if (top == nullptr)
    top = &m_topDiscCap;

  if (bottom == nullptr)
    bottom = &m_bottomDiscCap;

  m_topCap    = top;
  m_bottomCap = bottom;
  m_dirty     = true;
}

void
GLCappedCylinder::setRadius(GLdouble radius)
{
  m_radius = radius;
  m_topDiscCap.setRadius(radius);
  m_bottomDiscCap.setRadius(radius);
  m_dirty  = true;
}

void
GLCappedCylinder::setSlices(GLint slices)
{
  m_slices = slices;
  m_topDiscCap.setSlices(slices);
  m_bottomDiscCap.setSlices(slices);
  m_dirty  = true;
}

void
GLCappedCylinder::setVisibleCaps(bool base, bool top)
{
  m_drawBase = base;
  m_drawTop  = top;
  m_dirty    = true;
}

void
GLCappedCylinder::setInvertNormals(bool invert)
{
  m_invertNormals = invert;
  m_dirty = true;
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
    m_topCap->display();
    glPopMatrix();
  }

  if (m_drawBase) {
    glPushMatrix();
    glRotatef(180, 1, 0, 0);
    m_bottomCap->display();
    glPopMatrix();
  }
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glVertexPointer(3, GL_FLOAT,   3 * sizeof(GLfloat), m_strip.data());
  glNormalPointer(GL_FLOAT, 3 * sizeof(GLfloat), m_normals.data());
  glDrawArrays(GL_TRIANGLE_STRIP, 0, m_strip.size() / 3);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
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
// This sphere has its vertex on 0, 0, 0. This is, the sphere is tangent to
// the XY plane Z = 0. Since the radius of curvature R_c puts its center in
// Z = R_c, then we can do the following trick. Evaluate the z' of the
// sphere centered in 0, 0, 0:
//
// z' = sqrt(R_c^2 - x^2 - y^2)
//
// And calculate Z as:
//
// Z = R_c - z'
//
// R_c is essentially m_radius. T
//
//

void
GLSphericalCap::recalculate()
{
  GLint i, j, n    = 0, total;
  GLfloat angDelta = 2 * M_PI / m_sectors;
  GLfloat rDelta   = m_radius / m_stacks;
  GLfloat secDelta = 1.f / m_sectors;
  GLfloat stDelta  = 1.f / m_stacks;
  GLfloat Rc2      = m_rCurv * m_rCurv;
  GLfloat sign     = m_rCurv < 0 ? -1 : 1;
  GLfloat depth    = sign * sqrt(Rc2 - m_radius * m_radius);
  GLint k1, k2;

  total = (m_stacks + 1) * (m_sectors + 1);
  m_vertices.resize(3 * total);
  m_normals.resize(3 * total);
  m_texCoords.resize(2 * total);
  m_edge.resize(3 * m_sectors);

  GLfloat rInv = 1. / m_rCurv;
  if (m_invertNormals)
    rInv = -rInv;

  
  // This loop traverses the radius
  for (j = 0; j <= m_stacks; ++j) {
    GLfloat r = rDelta * j;

    // And this other one, the angles. In the case 
    for (i = 0; i <= m_sectors; ++i) {
      GLfloat ang = i * angDelta;
      
      GLfloat x = r * cosf(ang) + m_x0;
      GLfloat y = r * sinf(ang) + m_y0;
      GLfloat z = depth - sign * sqrt(Rc2 - x * x - y * y);

      //
      // The direction vector from the center is:
      // 
      // nx = (x - x_0) / R
      // ny = (y - y_0) / R
      // nz = (z - z_0) / R
      //
      // With z_0 = R_c
      //

      if (j == m_stacks && i < m_sectors) {
        m_edge[3 * i + 0] = x;
        m_edge[3 * i + 1] = y;
        m_edge[3 * i + 2] = z;
      }

      m_vertices[3 * n + 0] = x;
      m_vertices[3 * n + 1] = y;
      m_vertices[3 * n + 2] = z;

      m_normals[3 * n + 0] = (x - m_x0) * rInv;
      m_normals[3 * n + 1] = (y - m_y0) * rInv;
      m_normals[3 * n + 2] = (z - m_rCurv) * rInv;

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

  d2 = m_invertNormals ? 2 : 1;
  d1 = m_invertNormals ? 1 : 2;

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
GLSphericalCap::setCenterOffset(GLdouble x, GLdouble y)
{
  m_x0    = x;
  m_y0    = y;
  m_dirty = true;
}

const std::vector<GLfloat> *
GLSphericalCap::edge() const
{
  return &m_edge;
}

void
GLSphericalCap::requestRecalc()
{
  recalculate();
}

void
GLSphericalCap::setCurvatureRadius(GLdouble R)
{
  m_rCurv  = R;
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

//////////////////////////// GLParabolicCap //////////////////////////////////
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
  m_edge.resize(3 * m_sectors);

  // This loop traverses the radius
  for (j = 0; j <= m_stacks; ++j) {
    GLfloat r = rDelta * j;
    GLfloat nInv = 1. / sqrt(4 * k * k * r * r + 1);
    if (m_invertNormals)
      nInv = -nInv;

    // And this other one, the angles
    for (i = 0; i <= m_sectors; ++i) {
      GLfloat ang = i * angDelta;
      
      GLfloat x = r * cosf(ang) + m_x0;
      GLfloat y = r * sinf(ang) + m_y0;
      GLfloat z = k * (x * x + y * y - R2);

      if (j == m_stacks && i < m_sectors) {
        m_edge[3 * i + 0] = x;
        m_edge[3 * i + 1] = y;
        m_edge[3 * i + 2] = z;
      }

      m_vertices[3 * n + 0] = x;
      m_vertices[3 * n + 1] = y;
      m_vertices[3 * n + 2] = z;

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

const std::vector<GLfloat> *
GLParabolicCap::edge() const
{
  return &m_edge;
}

void
GLParabolicCap::requestRecalc()
{
  recalculate();
}

void
GLParabolicCap::setCenterOffset(GLdouble x, GLdouble y)
{
  m_x0 = x;
  m_y0 = y;
  m_dirty = true;
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

//////////////////////////// GLConicCap //////////////////////////////////
GLConicCap::GLConicCap()
{
  m_quadric   = gluNewQuadric();
  m_convex    = false;
  m_rCurv     = 1;
  m_K         = 0;
  m_radius    = 0.25;
  m_x0 = m_y0 = 0;
  m_rHole     = 0;
  m_sectors   = 64;
  m_stacks    = 8;
  m_invertNormals = false;
  m_dirty     = true;
}

GLConicCap::~GLConicCap()
{
  gluDeleteQuadric(m_quadric);
}

//
// Inspired in http://www.songho.ca/opengl/gl_sphere.html
//
void
GLConicCap::recalculate()
{
  GLint i, j, n    = 0, total;
  GLfloat angDelta = 2 * M_PI / m_sectors;
  GLfloat r0       = m_rHole;
  GLfloat rDelta   = (m_radius - r0) / m_stacks;
  GLfloat secDelta = 1.f / m_sectors;
  GLfloat stDelta  = 1.f / m_stacks;
  GLfloat R2       = m_radius * m_radius;
  GLint k1, k2;
  GLfloat sigma    = m_convex ? 1 : -1;
  GLfloat D;
  

  auto K1 = m_K + 1;
  auto invK1 = 1 / K1;
  auto inv2R = .5 / m_rCurv;
  bool parabola = isZero(K1);
  auto Rc2 = m_rCurv * m_rCurv;
  bool hasHole = m_rHole > 0;

  if (parabola) {
    D = .5 * R2 / m_rCurv;
  } else {
    D = (m_rCurv - sqrt(Rc2 - K1 * R2)) / K1;
  }

  auto RDKD = m_rCurv - D * (K1);

  total = (m_stacks + 1) * (m_sectors + 1);
  m_vertices.resize(3 * total);
  m_normals.resize(3 * total);
  m_texCoords.resize(2 * total);
  m_edge.resize(3 * m_sectors);

  // This loop traverses the radius
  for (j = 0; j <= m_stacks; ++j) {
    GLfloat r = rDelta * j + r0;

    // And this other one, the angles
    for (i = 0; i <= m_sectors; ++i) {
      GLfloat ang = i * angDelta;
      GLfloat x = r * cosf(ang) + m_x0;
      GLfloat y = r * sinf(ang) + m_y0;
      GLfloat rho2 = x * x + y * y;
      
      GLfloat z = parabola 
        ? -sigma * (inv2R * rho2 - D)
        : -sigma * (invK1 * (m_rCurv - sqrt(Rc2 - K1 * rho2)) - D);
          GLfloat dFdz = K1 * z + sigma * RDKD;

      GLfloat nInv = 1. / sqrt(rho2 + dFdz * dFdz);
      if (m_invertNormals)
        nInv = -nInv;

      if (j == m_stacks && i < m_sectors) {
        m_edge[3 * i + 0] = x;
        m_edge[3 * i + 1] = y;
        m_edge[3 * i + 2] = z;
      }

      m_vertices[3 * n + 0] = x;
      m_vertices[3 * n + 1] = y;
      m_vertices[3 * n + 2] = z;

      m_normals[3 * n + 0] = sigma * x * nInv;
      m_normals[3 * n + 1] = sigma * y * nInv;
      m_normals[3 * n + 2] = sigma * dFdz * nInv;

      m_texCoords[2 * n + 0] = i * secDelta;
      m_texCoords[2 * n + 1] = j * stDelta;

      ++n;
    }
  }

  // Calculate indices
  total = 2 * m_sectors * m_stacks - (hasHole ? 0 : m_sectors);
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
      if (hasHole || i != 0) {
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

const std::vector<GLfloat> *
GLConicCap::edge() const
{
  return &m_edge;
}

void
GLConicCap::requestRecalc()
{
  recalculate();
}

void
GLConicCap::setCenterOffset(GLdouble x, GLdouble y)
{
  m_x0 = x;
  m_y0 = y;
  m_dirty = true;
}

void
GLConicCap::setConicConstant(GLdouble K)
{
  m_K = K;
  m_dirty = true;
}

void
GLConicCap::setCurvatureRadius(GLdouble Rc)
{
  m_rCurv = Rc;
  m_dirty = true;
}

void
GLConicCap::setConvex(bool convex)
{
  m_convex = convex;
  m_dirty  = true;
}

void
GLConicCap::setRadius(GLdouble radius)
{
  m_radius = radius;
  m_dirty  = true;
}

void
GLConicCap::setHoleRadius(GLdouble radius)
{
  m_rHole = radius;
  m_dirty = true;
}

void
GLConicCap::setSectors(GLint slices)
{
  m_sectors = slices;
  m_dirty   = true;
}

void
GLConicCap::setStacks(GLint stacks)
{
  m_stacks = stacks;
  m_dirty  = true;
}

void
GLConicCap::setInvertNormals(bool inv)
{
  m_invertNormals = inv;
  m_dirty         = true;
}

void
GLConicCap::display()
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

///////////////////////////// GLAbstractCap ////////////////////////////////////
void
GLAbstractCap::requestRecalc()
{

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
  GLfloat zNorm = m_invertNormals ? -1 : 1;

  total = m_slices;

  // GL Disc consists of several triangles
  // The number of slices is the number of triangles
  m_vertices.resize (3 * 3 * total);
  m_normals.resize  (3 * 3 * total);
  m_texCoords.resize(2 * 3 * total);
  m_edge.resize(3 * m_slices);
  ang = 0;

  GLfloat x  = .5 * m_width  * cosf(ang);
  GLfloat y  = .5 * m_height * sinf(ang);

  for (i = 0; i < m_slices; ++i) {
    // Center
    m_vertices[3 * n + 0] = 0;
    m_vertices[3 * n + 1] = 0;
    m_vertices[3 * n + 2] = 0;

    m_normals[3 * n + 0] = 0;
    m_normals[3 * n + 1] = 0;
    m_normals[3 * n + 2] = zNorm;

    m_texCoords[2 * n + 0] = i * sliceDelta;
    m_texCoords[2 * n + 1] = 0;
    ++n;

    // Left, outer
    m_vertices[3 * n + 0] = x;
    m_vertices[3 * n + 1] = y;
    m_vertices[3 * n + 2] = 0;

    m_normals[3 * n + 0] = 0;
    m_normals[3 * n + 1] = 0;
    m_normals[3 * n + 2] = zNorm;

    m_texCoords[2 * n + 0] = i * sliceDelta;
    m_texCoords[2 * n + 1] = 1;
    ++n;

    // Right, outer
    ang += angDelta;

    x  = .5 * m_width  * cosf(ang);
    y  = .5 * m_height * sinf(ang);

    m_vertices[3 * n + 0] = x;
    m_vertices[3 * n + 1] = y;
    m_vertices[3 * n + 2] = 0;

    m_normals[3 * n + 0] = 0;
    m_normals[3 * n + 1] = 0;
    m_normals[3 * n + 2] = zNorm;

    m_texCoords[2 * n + 0] = (i + 1) * sliceDelta;
    m_texCoords[2 * n + 1] = 1;

    // Edge
    m_edge[3 * i + 0] = x;
    m_edge[3 * i + 1] = y;
    m_edge[3 * i + 2] = 0;
    
    ++n;
  }

  // Calculate indices
  n     = 0;
  m_indices.resize(3 * m_slices);

  if (m_invertNormals) {
    for (i = 0; i < m_slices; ++i) {
      m_indices[3 * i + 0] = 3 * i;
      m_indices[3 * i + 1] = 3 * i + 2;
      m_indices[3 * i + 2] = 3 * i + 1;
    }
  } else {
    for (i = 0; i < 3 * m_slices; ++i)
      m_indices[i] = i;
  }

  m_dirty = false;
}

const std::vector<GLfloat> *
GLDisc::edge() const
{
  return &m_edge;
}

void
GLDisc::requestRecalc()
{
  recalculate();
}

void
GLDisc::setInverted(bool inv)
{
  m_invertNormals = inv;
  m_dirty = true;
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
  m_normals.resize(stride);
  p = 0;

  for (j = 0; j < m_slices + 3; ++j) {
    m_normals[3 * j + 0] = 0;
    m_normals[3 * j + 1] = 0;
    m_normals[3 * j + 2] = 1;
  }

  for (j = 0; j < 4; ++j) {
    m_vertices[p++] = (m_height / 2) * sgn(sin(angle + M_PI / 4));
    m_vertices[p++] = (m_width  / 2) * sgn(cos(angle + M_PI / 4));
    
    m_vertices[p++] = 0;

    for (i = 0; i <= m_slices; ++i) {
      m_vertices[p++] = m_radius * sin(angle);
      m_vertices[p++] = m_radius * cos(angle);
      
      m_vertices[p++] = 0;

      if (i < m_slices)
        angle += angDelta;
    }

    m_vertices[p++] = (m_height  / 2) * sgn(sin(angle + M_PI / 4));
    m_vertices[p++] = (m_width   / 2) * sgn(cos(angle + M_PI / 4));
    
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

  glPushAttrib(GL_ENABLE_BIT);

  for (i = 0; i < 4; ++i) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, m_vertices.data() + i * stride);
    glNormalPointer(GL_FLOAT, 3 * sizeof(GLfloat), m_normals.data());
    glDrawArrays(GL_TRIANGLE_FAN, 0, stride / 3);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
  }

  glPopAttrib();
}

///////////////////////////////// GLRectangle //////////////////////////////////
GLRectangle::GLRectangle()
{
  m_vertices.resize(4);
}

GLRectangle::~GLRectangle()
{
  
}

void
GLRectangle::recalculate()
{
  m_vertices[0] = -.5 * m_width;
  m_vertices[1] = -.5 * m_height;

  m_vertices[2] = +.5 * m_width;
  m_vertices[3] = +.5 * m_height;

  m_dirty = false;
}

void
GLRectangle::setHeight(GLdouble height)
{
  m_height = height;
  m_dirty  = true;
}

void
GLRectangle::setWidth(GLdouble width)
{
  m_width = width;
  m_dirty = true;
}

void
GLRectangle::display()
{
  if (m_dirty)
    recalculate();

  glNormal3f(0, 0, 1.);
  glRectfv(m_vertices.data(), m_vertices.data() + 2);
}

/////////////////////////////// GLReferenceFrame ///////////////////////////////
void
GLReferenceFrame::setHeight(GLfloat height)
{
  m_height = height;
  m_axisCylinder.setHeight(height);
}

void
GLReferenceFrame::setRadius(GLfloat radius)
{
  m_radius = radius;
  m_axisCylinder.setRadius(radius);
}

void
GLReferenceFrame::setArrowHeight(GLfloat height)
{
  m_arrowHeight = height;
  m_axisArrow.setHeight(height);
}

void
GLReferenceFrame::setArrowBase(GLfloat base)
{
  m_arrowBase = base;
  m_axisArrow.setBase(base);
}

void
GLReferenceFrame::display()
{
  RZ::GLVectorStorage vec;

  glPushMatrix();
  
  glPushAttrib(GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glMaterialfv(GL_FRONT, GL_AMBIENT,  vec.get(0.1, 0.1, 0.1));

    // Z-axis
    glPushMatrix();
      glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(.208, .388, .788));
      glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, 0, 0));
      m_axisCylinder.display();
      glTranslatef(0, 0, m_height);
      m_axisArrow.display();
    glPopMatrix();

    // Y-axis
    glPushMatrix();
      glRotatef(-90, 1, 0, 0);
      glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(0.627, .984, .298));
      glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, 0, 0));
      m_axisCylinder.display();
      glTranslatef(0, 0, m_height);
      m_axisArrow.display();
    glPopMatrix();

    // X-axis
    glPushMatrix();
      glRotatef(+90, 0, 1, 0);

      glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(.902, .282, .106));
      glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, 0, 0));
      m_axisCylinder.display();
      glTranslatef(0, 0, m_height);
      m_axisArrow.display();
    glPopMatrix();
  glPopAttrib();
  glPopMatrix();
}

GLReferenceFrame::GLReferenceFrame()
{
  m_axisCylinder.setSlices(24);
  m_axisCylinder.setHeight(m_height);
  m_axisCylinder.setRadius(m_radius);
  m_axisCylinder.setVisibleCaps(true, false);
  
  m_axisArrow.setHeight(m_arrowHeight);
  m_axisArrow.setBase(m_arrowBase);
  m_axisArrow.setSlices(20);
  m_axisArrow.setStacks(20);
}

GLReferenceFrame::~GLReferenceFrame()
{

}

////////////////////////////////// GLArrow /////////////////////////////////////
void
GLArrow::setThickness(GLfloat thickness)
{
  m_thickness = thickness;
}

void
GLArrow::setDirection(Vec3 const &vec)
{
  m_direction = vec;
}

void
GLArrow::setOrigin(Vec3 const &origin)
{
  m_origin = origin;
}

void
GLArrow::display()
{
  glPushAttrib(GL_LINE_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR);
    glLineWidth(m_thickness);

    glBegin(GL_LINES);
      glVertex3f(m_origin.x, m_origin.y, m_origin.z);
      glVertex3f(
        m_origin.x + m_direction.x,
        m_origin.y + m_direction.y,
        m_origin.z + m_direction.z);
    glEnd();
  glPopAttrib();
}

GLArrow::GLArrow()
{

}

GLArrow::~GLArrow()
{

}

//////////////////////////////// GLGrid ////////////////////////////////////////

#define GLGRID_ANGLE_SEGS 36

void
GLGrid::recalculateHighlight()
{
  Real halfWidth = .5 * m_stepsX * m_step;

  m_hlVertices.clear();

  if (m_x * m_x + m_y * m_y > std::numeric_limits<GLfloat>::epsilon()) {
    Real R = sqrt(m_x * m_x + m_y * m_y);

    m_hlVertices.push_back(m_x);
    m_hlVertices.push_back(m_y);
    m_hlVertices.push_back(0);

    m_hlVertices.push_back(0);
    m_hlVertices.push_back(0);
    m_hlVertices.push_back(0);

    m_hlVertices.push_back(.3 * R < halfWidth ? halfWidth : .3 * R);
    m_hlVertices.push_back(0);
    m_hlVertices.push_back(0);

    Real theta = atan2(m_y, m_x);

    if (theta < 0)
      theta += 2 * M_PI;
    
    unsigned int steps = ceil(theta / (2 * M_PI) * GLGRID_ANGLE_SEGS);

    Real dTheta = theta / steps;
    
    R *= .3;

    for (unsigned int i = 0; i <= steps; ++i) {
      m_hlVertices.push_back(R * cos(i * dTheta));
      m_hlVertices.push_back(R * sin(i * dTheta));
      m_hlVertices.push_back(0);
    }
  }
}

void
GLGrid::recalculate()
{
  Real x0 = - .5 * m_stepsX * m_step;
  Real y0 = - .5 * m_stepsY * m_step;
  int i, j;

  m_vertices.clear();

  for (i = 0; i <= m_stepsX; ++i) {
    Real x = (i - .5 * m_stepsX) * m_step;

    m_vertices.push_back(x);
    m_vertices.push_back(y0);
    m_vertices.push_back(0);

    m_vertices.push_back(x);
    m_vertices.push_back(-y0);
    m_vertices.push_back(0);
  }
  
  for (j = 0; j <= m_stepsY; ++j) {
    Real y = (j - .5 * m_stepsY) * m_step;
    m_vertices.push_back(x0);
    m_vertices.push_back(y);
    m_vertices.push_back(0);
    
    m_vertices.push_back(-x0);
    m_vertices.push_back(y);
    m_vertices.push_back(0);
  }
}

void
GLGrid::highlight(GLfloat x, GLfloat y)
{
  if (!iszero(x - m_x) || !iszero(y - m_y)) {
    m_x = x;
    m_y = y;
    recalculateHighlight();
  }
}

void
GLGrid::setGridColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  m_gridColor[0] = r;
  m_gridColor[1] = g;
  m_gridColor[2] = b;
  m_gridColor[3] = a;
}

void
GLGrid::setHighlightColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  m_hlColor[0] = r;
  m_hlColor[1] = g;
  m_hlColor[2] = b;
  m_hlColor[3] = a;
}

void
GLGrid::display()
{
  glPushAttrib(GL_LINE_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR);
    glLineWidth(m_thickness);

    glColor4f(m_gridColor[0], m_gridColor[1], m_gridColor[2], m_gridColor[3]);
    glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_vertices.data());
      glDrawArrays(GL_LINES, 0, m_vertices.size() / 3);
    glDisableClientState(GL_VERTEX_ARRAY);

    glLineWidth(2 * m_thickness);
    if (m_hlVertices.size() > 0) {
      glColor4f(m_hlColor[0], m_hlColor[1], m_hlColor[2], m_hlColor[3]);
      glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_hlVertices.data());
        glDrawArrays(GL_LINE_STRIP, 0, m_hlVertices.size() / 3);
      glDisableClientState(GL_VERTEX_ARRAY);
    }
  glPopAttrib();
}

void
GLGrid::setStepsX(unsigned stepsX)
{
  m_stepsX = stepsX;
  recalculate();
}

void
GLGrid::setStepsY(unsigned stepsY)
{
  m_stepsY = stepsY;
  recalculate();
}

void
GLGrid::setStep(Real step)
{
  m_step = step;
  recalculate();
}

void
GLGrid::setThickness(GLfloat thickness)
{
  m_thickness = thickness;
}

GLGrid::GLGrid()
{
  recalculate();
}

GLGrid::~GLGrid()
{

}

//////////////////////////////////// GLText ////////////////////////////////////
void
GLText::composeTexture()
{
  unsigned int x = 0, height = 0, advance = 0, charWidth = 0;
  int x0, y0, x1, y1;
  int ymin = 0, ymax = 0, xmin = 0, xmax = 0;

  FT2Facade *ft = Singleton::instance()->freetype();
  FT_Error error;
  FT_Face face = ft->loadFace(m_face, error);

  m_texLoaded = false;
  m_needsReload = true;

  if (m_text.empty())
    return;
  
  FT_Set_Pixel_Sizes(face, 0, m_fontSize);

  // Dry run: estimate texture size
  for (auto i = 0; i < m_text.size(); ++i) {
    x += advance;

    if (FT_Load_Char(face, m_text[i], FT_LOAD_RENDER)) {
      RZError("FreeType2: cannot load glyph for ASCII = %d\n", m_text[i]);
      goto done;
    }

    charWidth = face->glyph->bitmap.width;

    // Calculate bounding box. The interpretation of the bitmap_top and
    // height is a bit tricky: it means that the character starts at `bitmap_top`
    // vertically upwards, and develops itself `rows` downards. This means that:
    //
    // ymin = top - height
    // ymax = ymin + height = top
    x0 = x + face->glyph->bitmap_left;
    y0 = face->glyph->bitmap_top - face->glyph->bitmap.rows;
    x1 = x0 + charWidth;
    y1 = face->glyph->bitmap_top;

    if (x0 < xmin)
      xmin = x0;
    if (y0 < ymin)
      ymin = y0;
    if (x1 > xmax)
      xmax = x1;
    if (y1 > ymax)
      ymax = y1;

    advance = face->glyph->advance.x >> 6;
  }

  m_texWidth  = xmax - xmin;
  m_texHeight = ymax - ymin;

  // Allocate texture and load glyphs
  m_texture.resize(m_texWidth * m_texHeight);
  memset(m_texture.data(), 0, m_texture.size() * sizeof(uint32_t));

  x = 0;
  advance = 0;

  for (auto i = 0; i < m_text.size(); ++i) {
    x += advance;

    if (FT_Load_Char(face, m_text[i], FT_LOAD_RENDER)) {
      RZError("FreeType2: cannot load glyph for ASCII = %d\n", m_text[i]);
      goto done;
    }

    advance = face->glyph->advance.x >> 6;

    auto glyphWidth  = face->glyph->bitmap.width;
    auto glyphHeight = face->glyph->bitmap.rows;

    // Calculate bounding box
    x0 = x + face->glyph->bitmap_left;
    y0 = face->glyph->bitmap_top - glyphHeight;
    
    for (auto j = 0; j < glyphHeight; ++j) {
      for (auto i = 0; i < glyphWidth; ++i) {
        m_texture[x0 + i + (j + (y0 - ymin)) * m_texWidth] = 
          0xffffff00 | face->glyph->bitmap.buffer[i + (glyphHeight - j - 1) * glyphWidth];
      }
    }
  }

  m_haveTexture = true;

done:
  if (m_haveTexture) {
    // Define vertices and finish
    GLfloat vertices[6][2] = {
          { 0,                    m_scale * m_texHeight},
          { 0,                    0                    },
          { m_scale * m_texWidth, 0                    },
          { 0,                    m_scale * m_texHeight},
          { m_scale * m_texWidth, 0                    },
          { m_scale * m_texWidth, m_scale * m_texHeight}};

    memcpy(m_vertices, vertices, sizeof(vertices));
  }
}

void
GLText::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  m_color[0] = r;
  m_color[1] = g;
  m_color[2] = b;
  m_color[3] = a;
}

void
GLText::setSize(unsigned size)
{
  if (m_fontSize != size) {
    m_fontSize = size;
    composeTexture();
  }
}

void
GLText::setScale(GLfloat scale)
{
  m_scale = scale;
  composeTexture();
}

void
GLText::setText(std::string const &text)
{
  if (text != m_text) {
    m_text = text;
    composeTexture();
  }
}

void
GLText::setFace(std::string const &face)
{
  if (face != m_face) {
    m_face = face;
    composeTexture();
  }
}

void
GLText::reloadTexture()
{
  if (!m_haveTexture)
    return;
  
  if (m_texLoaded) {
    glDeleteTextures(1, &m_texId);
    m_texLoaded = false;
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
  
  glGenTextures(1, &m_texId);
  glBindTexture(GL_TEXTURE_2D, m_texId);

  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA8,
      m_texWidth,
      m_texHeight,
      0,
      GL_RGBA,
      GL_UNSIGNED_INT_8_8_8_8,
      m_texture.data()
  );

  // set texture options
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  m_texLoaded = true;

done:
  return;
}

GLText::~GLText()
{

}

static const GLfloat g_textTexCoords[6][2] = {
  {0, 1},
  {0, 0},
  {1, 0},
  {0, 1},
  {1, 0},
  {1, 1}
};

void
GLText::display()
{
  GLfloat x = 0;

  if (m_needsReload) {
    reloadTexture();
    m_needsReload = false;
  }

  if (!m_texLoaded)
    return;

  glPushAttrib(GL_LINE_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    glActiveTexture(GL_TEXTURE0);
    glColor4fv(m_color);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

      glTexCoordPointer(2, GL_FLOAT, 2 * sizeof(float), g_textTexCoords);
      glVertexPointer(2, GL_FLOAT, 2 * sizeof(float), m_vertices);

      glBindTexture(GL_TEXTURE_2D, m_texId);
      glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glBindTexture(GL_TEXTURE_2D, 0);
  glPopAttrib();
}
