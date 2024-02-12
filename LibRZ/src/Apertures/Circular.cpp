#include <Apertures/Circular.h>

using namespace RZ;

CircularAperture::CircularAperture(Real radius)
{
  setRadius(radius);
}

void
CircularAperture::setRadius(Real radius)
{
  m_radius2 = radius * radius;
}

bool
CircularAperture::intercept(Vec3 &coord, Vec3 &n, Vec3 const &) const
{ 
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
  }
}
