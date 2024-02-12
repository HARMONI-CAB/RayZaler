#include <Apertures/Spherical.h>

using namespace RZ;

SphericalAperture::SphericalAperture(Real R, Real rCurv)
{
  setRadius(R);
  setCurvatureRadius(rCurv);  
}

void
SphericalAperture::recalcDistribution()
{
  m_rCurv2 = m_rCurv * m_rCurv;
  m_K      = 1. / (m_rCurv2 - m_rCurv * sqrt(m_rCurv2 - m_radius2));
  m_KRcInv = 1. / (m_K * m_rCurv);
}

void
SphericalAperture::setRadius(Real R)
{
  m_radius2 = R * R;
  m_center = sqrt(m_rCurv * m_rCurv - m_radius2);
  recalcDistribution();
}

void
SphericalAperture::setConvex(bool convex)
{
  m_convex = convex;
}

void
SphericalAperture::setCurvatureRadius(Real rCurv)
{
  m_rCurv = rCurv;
  m_center = sqrt(m_rCurv * m_rCurv - m_radius2);
  recalcDistribution();
}

//
// Rays always arrive with positive z. This means that origins are always
// with negative z. Anything with positive z is a back ray and hence is wrong.
//
// Convex lens (B) have their center with z > 0. Concave lens (A), with z < 0.
// Spherical lens processors describe the intersection of a ray on something
// constructed like this:
//
//  (A)              (B)
//   |                |
//   |\              /|
//   | |     or     | |
//   | |    this    | |
//   |/              \|
//   |                |
//  InS              Ins
//
// -----> Direction of propagation ---> (+z)
//
//   Lens bumps a distance d from the intercept surface, which means
//   that the curvature center is a distance Rc - d to the left. In the
//   edge of the lens, the curvature center is exactly at a distance Rc, and
//   at a distance R to the vertical distance to the center of the lens. This
//   means that:
//   
//   R = Rc * sin(alpha)
//
//   On the other hand, cos(alpha) = (Rc - d) / Rc = 1 - d / Rc
//
//   Since cos^2 + sin^2 = 1, then:
//
//   R^2 = Rc^2 (1 - sin^2) = Rc^2 * (1 - (1 - d/Rc)^2) =
//       = Rc^2 (1 - 1 - d^2/Rc^2 + 2d/Rc)
//       = Rc^2 (2d/Rc - d^2 / Rc^2) = 
//       = 2dRc - d^2
//  
//   We have to solve for d:
//
//   d^2 - 2Rc d + R^2 = 0
//
//   There are two solutions for d:
//
//   d1 = Rc + sqrt(Rc^2 - R^2)
//   d2 = Rc - sqrt(Rc^2 - R^2)
//
//   And conversely, for the center of curvature, in the propagation axis:
//
//   C1 = - sqrt(Rc^2 - R^2)
//   C2 = + sqrt(Rc^2 - R^2)
//
//   In case A, the center of curvature is C1. In case B, it is C2
//
//   Now, when we compute the intersection, we are going to have 2 origin-relative
//   solutions for t. t1 < t2. t1 is the convex one, and t2 is the concave one.
//  

bool
SphericalAperture::intercept(
  Vec3 &coord,
  Vec3 &normal,
  Vec3 const &origin) const
{
  auto u    = (coord - origin).normalized();
  Real sign = m_convex ? +1 : -1;
  auto Cs   = sign * m_center * Vec3::eZ();
  auto xC   = origin - Cs;
  
  if (u.z < 0)
    sign = -sign;
  
  if (coord.x * coord.x + coord.y * coord.y < m_radius2) {
    // Real a = u * u = 1
    Real b = xC * u;
    Real c = xC * xC - m_rCurv * m_rCurv;

    // Discriminat. Does the ray hit the aperture?
    Real D = b * b - c;

    if (D >= 0) {
      Real sqrtD = sqrt(D);

      // In the convex case, we return the + case. In the concave case,
      // we return the - case
      Real t = - b - sign * sqrtD;
      
      // Adjust hit point
      coord = origin + t * u;

      // Calculate the interception normal
      normal = sign * (coord - Cs).normalized();
      
      return true;
    }
  }

  return false;
}

void
SphericalAperture::generatePoints(
    const ReferenceFrame *frame,
    Real *pointArr,
    unsigned int N)
{
  unsigned int i;
  Real x, y, z;
  Real alpha, rho, u, rad;
  auto &state = randState();

  for (i = 0; i < N; ++i) {
    alpha = 2 * M_PI * state.randu();
    u     = state.randu();

    rad   = m_rCurv - u * m_KRcInv;
    rho   = sqrt(m_rCurv2 - rad * rad);

    x     = rho * cos(alpha);
    y     = rho * sin(alpha);
    z     = m_rCurv - sqrt(m_rCurv2 - rho * rho) - m_center;

    if (!m_convex)
      z = -z;

    frame->fromRelative(Vec3(x, y, z)).copyToArray(pointArr + 3 * i);
  }
}
