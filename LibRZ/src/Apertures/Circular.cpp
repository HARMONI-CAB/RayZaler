#include <Apertures/Circular.h>
#include <Logger.h>

using namespace RZ;

CircularAperture::CircularAperture(Real radius)
{
  setRadius(radius);
}

void
CircularAperture::recalculate()
{
  Real theta = 0;
  Real dTheta = 2 * M_PI / GENERIC_APERTURE_NUM_SEGMENTS;

  m_vertices.clear();

  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS; ++i) {
    GLfloat x = m_radius * m_a * cos(theta);
    GLfloat y = m_radius * m_b * sin(theta);
    
    m_vertices.push_back(x);
    m_vertices.push_back(y);
    m_vertices.push_back(0);

    theta += dTheta;
  }
}

void
CircularAperture::setRadius(Real radius)
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
CircularAperture::setEccentricity(Real ecc)
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
CircularAperture::intercept(
  Vec3 &coord,
  Vec3 &n,
  Real &deltaT,
  Vec3 const &) const
{ 
  deltaT = 0.;

  return coord.x * coord.x / m_a2 + coord.y * coord.y / m_b2 < m_radius2;
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
CircularAperture::generatePoints(
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
CircularAperture::area() const
{
  // a and b properly normalized, no need to care about them.
  return M_PI * m_radius2;
}

void
CircularAperture::renderOpenGL()
{
  glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_vertices.data());
    glDrawArrays(GL_LINE_LOOP, 0, m_vertices.size() / 3);
  glDisableClientState(GL_VERTEX_ARRAY);

  glBegin(GL_LINES);
    glVertex3f(-m_radius * m_a, 0, 0);
    glVertex3f(+m_radius * m_a, 0, 0);
    glVertex3f(0, -m_radius * m_b, 0);
    glVertex3f(0, +m_radius * m_b, 0);
  glEnd();
}
