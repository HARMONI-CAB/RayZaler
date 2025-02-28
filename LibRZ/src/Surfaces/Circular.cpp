#include <Surfaces/Circular.h>
#include <Logger.h>

using namespace RZ;

CircularFlatSurface::CircularFlatSurface(Real radius)
{
  m_edges.resize(1);
  setRadius(radius);
}

std::vector<std::vector<Real>> const &
CircularFlatSurface::edges() const
{
  return m_edges;
}

void
CircularFlatSurface::recalculate()
{
  Real theta  = 0;
  Real dTheta = 2 * M_PI / GENERIC_APERTURE_NUM_SEGMENTS;
  unsigned int N = (GENERIC_APERTURE_NUM_GRIDLINES - 1) / 2;
  Real dR        = 2 * m_radius / (GENERIC_APERTURE_NUM_GRIDLINES - 1);
  Real R2;
  m_vertices.clear();
  m_grid.clear();

  m_edges[0].clear();

  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS; ++i) {
    GLfloat x = m_radius * m_a * cos(theta);
    GLfloat y = m_radius * m_b * sin(theta);
    
    m_vertices.push_back(x);
    m_vertices.push_back(y);
    m_vertices.push_back(0);

    m_edges[0].push_back(x);
    m_edges[0].push_back(y);
    m_edges[0].push_back(0);

    theta += dTheta;
  }

  if (m_obstruction) {
    GLfloat tmp;
    //
    // x^2 = sqrt(R^2 - y^2)
    //

    R2  = m_radius;
    R2 *= R2;

    for (unsigned i = 0; i < N; ++i) {
      GLfloat y = i * dR;
      GLfloat x = sqrt(R2 - y * y);

      // Horizontal sense: multiply x coordinates by m_a, y coordinates by m_b
      m_grid.push_back(+x * m_a);
      m_grid.push_back(+y * m_b);
      m_grid.push_back(0);
      
      m_grid.push_back(-x * m_a);
      m_grid.push_back(+y * m_b);
      m_grid.push_back(0);

      m_grid.push_back(+x * m_a);
      m_grid.push_back(-y * m_b);
      m_grid.push_back(0);
      
      m_grid.push_back(-x * m_a);
      m_grid.push_back(-y * m_b);
      m_grid.push_back(0);

      // Vertical sense. Same idea but exchanging x and y
      tmp = x;
      x = y;
      y = tmp;
      
      m_grid.push_back(+x * m_a);
      m_grid.push_back(-y * m_b);
      m_grid.push_back(0);
      
      m_grid.push_back(+x * m_a);
      m_grid.push_back(+y * m_b);
      m_grid.push_back(0);

      m_grid.push_back(-x * m_a);
      m_grid.push_back(-y * m_b);
      m_grid.push_back(0);
      
      m_grid.push_back(-x * m_a);
      m_grid.push_back(+y * m_b);
      m_grid.push_back(0);
    }
  }
}

void
CircularFlatSurface::setRadius(Real radius)
{
  m_radius  = radius;
  m_radius2 = radius * radius;

  recalculate();
}

// Circular apertures can be eccentric to define elliptical apertures.
// We assume that the equation for the circular aperture is:
//
//   x^2/a + y^2/b = R^2
//
// And we force ab = 1, i.e. a = 1 / b. We do this to make sure that the
// adjustments in eccentricity do not alter the area of the mirror.
//
// This has implications to the definition of eccentricity. In particular:
//
// e = +sqrt(1 - b^2/a^2) = +sqrt(1 - b^4) (if b < a)
// e = -sqrt(1 - a^2/b^2) = -sqrt(1 - a^4) (if a < b)
// e = 0                                   (if a = b)
// This means that:
//
// e^2 = 1 - b^4 => b^2 = sqrt(1 - e^2) (if e > 0)
// e^2 = 1 - a^4 => a^2 = sqrt(1 - e^2) (if e < 0)
// e^2 = 0       => a^2 = b^2 = 1       (if e = 0)
//
// Note that the sign is not in the traditional definition of eccentricity. We
// leverage it to determine which axis is elongated and which one is srhunk.
// 
//

void
CircularFlatSurface::setEccentricity(Real ecc)
{
  if (ecc <= -1 || ecc >= 1) {
    RZError("Eccentricity out of bounds\n");
    return;
  }

  if (ecc > 0) {
    m_b2 = sqrt(1 - ecc * ecc);
    m_a2 = 1 / m_b2;
  } else {
    m_a2 = sqrt(1 - ecc * ecc);
    m_b2 = 1 / m_a2;
  }

  m_a = sqrt(m_a2);
  m_b = sqrt(m_b2);

  recalculate();
}

bool
CircularFlatSurface::intercept(
  Vec3 &coord,
  Vec3 &n,
  Real &deltaT,
  Vec3 const &origin,
  Vec3 const &direction) const
{
  if (isZero(direction.z))
    return false;

  deltaT = -origin.z / direction.z;
  coord  = origin + deltaT * direction;
  n      = Vec3::eZ();
  
  if (coord.x * coord.x / m_a2 + coord.y * coord.y / m_b2 < m_radius2)
    return true;

  return false;
}

//
// This is the easiest way to sample points uniformly from a circular
// distribution:
//
// - Draw angle from U(0, 2pi)
// - Draw radius from R * sqrt(U(0, 1))
//
// The rationale of this sqrt is as follows: in this coordinate system,
// the area of each infinitesimal part is given by:
//
// dA = dr * r dphi = r * (dr * dphi)
//
// I.e. there is a scaling factor of r. 

void
CircularFlatSurface::generatePoints(
    const ReferenceFrame *frame,
    Real *pointArr,
    Real *normalArr,
    unsigned int N)
{
  Real theta, R = sqrt(m_radius2);
  Real r, x, y;

  auto &state = randState();
  unsigned int i;

  for (i = 0; i < N; ++i) {
    theta = 2 * M_PI * state.randu();
    r     = R * sqrt(state.randu());
    x     = r * m_a * cos(theta);
    y     = r * m_b * sin(theta);

    frame->fromRelative(Vec3(x, y, 0)).copyToArray(pointArr + 3 * i);
    frame->fromRelativeVec(Vec3(0, 0, 1)).copyToArray(normalArr + 3 * i);
  }
}

Real
CircularFlatSurface::area() const
{
  // a and b properly normalized, no need to care about them.
  return M_PI * m_radius2;
}

std::string
CircularFlatSurface::name() const
{
  return "CircularFlat";
}

void
CircularFlatSurface::setObstruction(bool obs)
{
  m_obstruction = obs;

  recalculate();
}

void
CircularFlatSurface::renderOpenGLAperture()
{
  glBegin(GL_LINES);
    glVertex3f(-m_radius * m_a, 0, 0);
    glVertex3f(+m_radius * m_a, 0, 0);
    glVertex3f(0, -m_radius * m_b, 0);
    glVertex3f(0, +m_radius * m_b, 0);
  glEnd();
}

void
CircularFlatSurface::renderOpenGLObstruction()
{
  glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_grid.data());
    glDrawArrays(GL_LINES, 0, m_grid.size() / 3);
  glDisableClientState(GL_VERTEX_ARRAY);
}

void
CircularFlatSurface::renderOpenGL()
{
  glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_vertices.data());
    glDrawArrays(GL_LINE_LOOP, 0, m_vertices.size() / 3);
  glDisableClientState(GL_VERTEX_ARRAY);

  if (m_obstruction)
    renderOpenGLObstruction();
  else
    renderOpenGLAperture();
}
