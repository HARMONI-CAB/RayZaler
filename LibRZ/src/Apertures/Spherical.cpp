#include <Apertures/Spherical.h>

using namespace RZ;

SphericalAperture::SphericalAperture(Real R, Real rCurv)
{
  setRadius(R);
  setCurvatureRadius(rCurv);
  recalcGL();
}

void
SphericalAperture::setCenterOffset(Real x, Real y)
{
  m_x0 = x;
  m_y0 = y;

  recalcDistribution();
  recalcGL();
}

//
// z  = m_center - sqrt(m_rCurv * m_rCurv - x * x);
//

void
SphericalAperture::recalcGL()
{
  GLfloat x, y, z, r, dh;
  Real theta = 0;
  Real dTheta = 2 * M_PI / GENERIC_APERTURE_NUM_SEGMENTS;
  Real sign = m_rCurv > 0 ? 1 : -1;
  
  m_vertices.clear();

  // Draw the aperture ellipse
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS; ++i) {
    x = m_radius * cos(theta) + m_x0;
    y = m_radius * sin(theta) + m_y0;
    z = m_center - sqrt(m_rCurv2 - x * x - y * y);

    m_vertices.push_back(x);
    m_vertices.push_back(y);
    m_vertices.push_back(sign * z);

    theta += dTheta;
  }

  m_axes.clear();

  dh = 2 * m_radius / GENERIC_APERTURE_NUM_SEGMENTS;

  r = -m_radius;
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS + 1; ++i) {
    x = m_ux * r + m_x0;
    y = m_uy * r + m_y0;
    z = m_center - sqrt(m_rCurv2 - x * x - y * y);

    m_axes.push_back(x);
    m_axes.push_back(y);
    m_axes.push_back(sign * z);

    r += dh;
  }

  r = -m_radius;
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS + 1; ++i) {
    x = -m_uy * r + m_x0;
    y = m_ux * r + m_y0;
    z = m_center - sqrt(m_rCurv2 - x * x - y * y);

    m_axes.push_back(x);
    m_axes.push_back(y);
    m_axes.push_back(sign * z);

    r += dh;
  }
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
  m_radius  = R;
  m_radius2 = R * R;
  m_center = sqrt(m_rCurv * m_rCurv - m_radius2);
  recalcDistribution();
  recalcGL();
}

void
SphericalAperture::setConvex(bool convex)
{
  m_convex = convex;
  recalcGL();
}

void
SphericalAperture::setCurvatureRadius(Real rCurv)
{
  m_rCurv = rCurv;
  m_center = sqrt(m_rCurv * m_rCurv - m_radius2);
  recalcDistribution();
  recalcGL();
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
  Real &deltaT,
  Vec3 const &origin) const
{
  auto u    = (coord - origin).normalized();
  Real sign = m_convex ? +1 : -1;
  auto Cs   = sign * m_center * Vec3::eZ();
  auto xC   = origin - Cs;
  
  if (u.z < 0)
    sign = -sign;
  
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
    deltaT = t;
    
    Real x = coord.x - m_x0;
    Real y = coord.y - m_y0;

    if (x * x + y * y >= m_radius2)
      return false;

    // Calculate the interception normal
    normal = sign * (coord - Cs).normalized();
    
    return true;
  }

  return false;
}

void
SphericalAperture::generatePoints(
    const ReferenceFrame *frame,
    Real *pointArr,
    Real *normalArr,
    unsigned int N)
{
  unsigned int i;
  Vec3 p, n, dfdr, dfda;
  Real cosA, sinA;
  Real alpha, rho, u, rad;
  auto &state = randState();

  dfda.z = 0;

  for (i = 0; i < N; ++i) {
    alpha = 2 * M_PI * state.randu();
    u     = state.randu();

    rad   = m_rCurv - u * m_KRcInv;
    rho   = sqrt(m_rCurv2 - rad * rad);

    cosA = cos(alpha);
    sinA = sin(alpha);

    if (!isZero(rho)) {
      p.x   = rho * cosA;
      p.y   = rho * sinA;
      p.z   = m_rCurv - sqrt(m_rCurv2 - rho * rho) - m_center;

      dfda.x = -rho * sinA;
      dfda.y =  rho * cosA;

      dfdr   = (Vec3(0, 0, m_rCurv - m_center) - p).normalized();

      n = dfdr.cross(dfda).normalized();  
    } else {
      n = Vec3::eZ();
    }

    if (!m_convex) {
      p.z = -p.z;
      n.z = -n.z;
    }

    frame->fromRelative(p).copyToArray(pointArr + 3 * i);
    frame->fromRelativeVec(n).copyToArray(normalArr + 3 * i);
  }
}

//
// The area of the spherical aperture is just the area of a spherical cap,
// which is given by:
//
// A = 2 pi Rc h
//
// And the height of the spherical cap is just the point at which the aperture
// ends, which is given by:
//
// h = Rc - sqrt(Rc^2 - Rmax^2) = Rc - m_center
//

Real
SphericalAperture::area() const
{
  return 2 * M_PI * m_rCurv * (m_rCurv - m_center);
}

void
SphericalAperture::renderOpenGL()
{
  auto N = m_axes.size() / (2 * 3);

  glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_vertices.data());
    glDrawArrays(GL_LINE_LOOP, 0, m_vertices.size() / 3);
  
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_axes.data());
    glDrawArrays(GL_LINE_STRIP, 0, N);

    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_axes.data() + 3 * N);
    glDrawArrays(GL_LINE_STRIP, 0, N);
  glDisableClientState(GL_VERTEX_ARRAY);
}
