#include <Apertures/Circular.h>

using namespace RZ;

CircularAperture::CircularAperture(Real radius)
{
  setRadius(radius);
}

void
CircularAperture::setRadius(Real radius)
{
  Real theta = 0;
  Real dTheta = 2 * M_PI / GENERIC_APERTURE_NUM_SEGMENTS;

  m_radius  = radius;
  m_radius2 = radius * radius;

  m_vertices.clear();

  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS; ++i) {
    GLfloat x = radius * cos(theta);
    GLfloat y = radius * sin(theta);
    
    m_vertices.push_back(x);
    m_vertices.push_back(y);
    m_vertices.push_back(0);

    theta += dTheta;
  }
}

bool
CircularAperture::intercept(
  Vec3 &coord,
  Vec3 &n,
  Real &deltaT,
  Vec3 const &) const
{ 
  deltaT = 0.;

  return coord.x * coord.x + coord.y * coord.y < m_radius2;
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
    x     = r * cos(theta);
    y     = r * sin(theta);

    frame->fromRelative(Vec3(x, y, 0)).copyToArray(pointArr + 3 * i);
    frame->fromRelativeVec(Vec3(0, 0, 1)).copyToArray(normalArr + 3 * i);
  }
}

Real
CircularAperture::area() const
{
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
    glVertex3f(-m_radius, 0, 0);
    glVertex3f(+m_radius, 0, 0);
    glVertex3f(0, -m_radius, 0);
    glVertex3f(0, +m_radius, 0);
  glEnd();
}
