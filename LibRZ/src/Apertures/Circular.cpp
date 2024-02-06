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


    Vec3 point = frame->eX() * x + frame->eY() * y + frame->getCenter();
    pointArr[3 * i + 0] = point.x;
    pointArr[3 * i + 1] = point.y;
    pointArr[3 * i + 2] = point.z;
  }
}
