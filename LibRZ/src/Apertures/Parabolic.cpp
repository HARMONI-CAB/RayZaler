#include <Apertures/Parabolic.h>

using namespace RZ;

ParabolicAperture::ParabolicAperture(Real R, Real fLength)
{
  setRadius(R);
  setFocalLength(fLength);
  recalcGL();
}

void
ParabolicAperture::recalcDistribution()
{
  Real K;
  Real x2, y2, invnorm;

  m_4f2   =  4 * m_flength * m_flength;
  m_8f3   = -2 * m_flength * m_4f2;
  K       = -6 * m_flength / (pow(m_4f2 + m_radius2, 1.5) - m_8f3);
  m_6f_K  = -6 * m_flength / K;
  m_depth = m_radius2 / (4 * m_flength);

  x2 = m_x0 * m_x0;
  y2 = m_y0 * m_y0;

  if (isZero(x2 + y2)) {
    m_ux = 1;
    m_uy = 0;
  } else {
    invnorm = 1 / sqrt(x2 + y2);
    m_ux = invnorm * m_x0;
    m_uy = invnorm * m_y0;
  }
}

void
ParabolicAperture::setCenterOffset(Real x, Real y)
{
  m_x0 = x;
  m_y0 = y;

  recalcDistribution();
  recalcGL();
}

void
ParabolicAperture::setRadius(Real R)
{
  m_radius  = R;
  m_radius2 = R * R;

  recalcDistribution();
  recalcGL();
}

void
ParabolicAperture::setFocalLength(Real fLength)
{
  m_flength = fLength;
  
  recalcDistribution();
  recalcGL();
}

void
ParabolicAperture::recalcGL()
{
  GLfloat x, y, z, r, dh;
  Real theta = 0;
  Real dTheta = 2 * M_PI / GENERIC_APERTURE_NUM_SEGMENTS;

  m_vertices.clear();

  auto K = 1 / (4 * m_flength);

  // Draw the aperture ellipse
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS; ++i) {
    x = m_radius * cos(theta) + m_x0;
    y = m_radius * sin(theta) + m_y0;
    z  = -K * (x * x + y * y) + m_depth;

    m_vertices.push_back(x);
    m_vertices.push_back(y);
    m_vertices.push_back(z);

    theta += dTheta;
  }

  m_axes.clear();

  dh = 2 * m_radius / GENERIC_APERTURE_NUM_SEGMENTS;

  r = -m_radius;
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS + 1; ++i) {
    x = m_ux * r + m_x0;
    y = m_uy * r + m_y0;
    z  = -K * (x * x + y * y) + m_depth;

    m_axes.push_back(x);
    m_axes.push_back(y);
    m_axes.push_back(z);

    r += dh;
  }

  r = -m_radius;
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS + 1; ++i) {
    x = -m_uy * r + m_x0;
    y = m_ux * r + m_y0;
    z  = -K * (x * x + y * y) + m_depth;

    m_axes.push_back(x);
    m_axes.push_back(y);
    m_axes.push_back(z);

    r += dh;
  }
}

bool
ParabolicAperture::intercept(
  Vec3 &intercept,
  Vec3 &normal,
  Real &deltaT,
  Vec3 const &Ot) const
{
  Vec3 ut = (intercept - Ot).normalized();

  if (Ot.z < 0)
    return false;

  Real x = Ot.x;
  Real y = Ot.y;
  Real z = Ot.z;

  Real a = ut.x;
  Real b = ut.y;
  Real c = ut.z;

  // Now, we solve a quadratic equation of the form:
  //
  // At^2 + Bt + C = 0

  // Coefficient A: just the projection of the ray on the XY plane
  Real A     = a * a + b * b;

  // Equation B: this is just the rect equation
  Real B     = 2 * (a * x + b * y + 2 * m_flength * c);
  Real C     = x * x + y * y - m_radius2 + 4 * m_flength * z;
  Real Delta = B * B - 4 * A * C;
  Real t;

  if (A <= std::numeric_limits<Real>::epsilon())
    t = -C / B;
  else if (Delta >= 0)
    t = .5 * (-B + sqrt(Delta)) / A;
  else
    return false;

  deltaT = t;
  
  // Vignetting test is performed only after computing the intersection with
  // the paraboloid.

  intercept = Ot + t * ut;
  x = intercept.x - m_x0;
  y = intercept.y - m_y0;

  if (x * x + y * y >= m_radius2)
    return false;

  normal = Vec3(
    0.5 * intercept.x / m_flength,
    0.5 * intercept.y / m_flength,
    1.0).normalized();

  return true;
}

void
ParabolicAperture::generatePoints(
    const ReferenceFrame *frame,
    Real *pointArr,
    Real *normalArr,
    unsigned int N)
{
  unsigned int i;
  Vec3 p;
  Vec3 n;
  Vec3 dfda;
  Vec3 dfdr;
  Real cosA, sinA;

  Real alpha, rho, u, rad, rho2;
  auto &state = randState();

  dfda.z = 0;

  for (i = 0; i < N; ++i) {
    alpha = 2 * M_PI * state.randu();
    u     = state.randu();

    rad   = pow(m_6f_K * u + m_8f3, 2. / 3.);
    rho2  = rad - m_4f2;
    rho   = sqrt(rho2);
    
    cosA = cos(alpha);
    sinA = sin(alpha);

    // This is our random point
    p.x = rho * cosA;
    p.y = rho * sinA;
    p.z = m_depth - rho2 / (4 * m_flength);

    // Surface normal is given by df/dr x df/da

    if (!isZero(rho)) {
      dfda.x = -rho * sinA;
      dfda.y =  rho * cosA;
      
      dfdr.x = cosA;
      dfdr.y = sinA;
      dfdr.z = -rho / (2 * m_flength);

      n = dfdr.cross(dfda).normalized();
    } else {
      n = Vec3::eZ();
    }
    
    frame->fromRelative(p).copyToArray(pointArr + 3 * i);
    frame->fromRelativeVec(n).copyToArray(normalArr + 3 * i);
  }
}

//
// The area of the paraboloid can be found by integrating the differential
// area elements, that are given by:
// 
// dA = rho * da * dl
//
// Where da is the angle differential and dl an infinitesimal element of the
// parabola. This infinitesimal element has the property:
//
// dl^2 = drho^2 + dz^2
//
// Where dz is given by the parabola function f(rho) = rho^2/(4F). Hence:
//
// dl^2 = drho^2 + ((df/drho) * drho)^2 = drho^2(1 + (rho/2F)^2)
//
// From which we conclude that dl = sqrt(1 + rho^2/4F^2) * drho which, once
// multiplied by rho in the dA expression, is easily integrable.
//

Real
ParabolicAperture::area() const
{
  Real A = 2. * M_PI * m_4f2 / 3. * (pow(1 + m_radius2 / m_4f2, 1.5) - 1.);
  return A;
}

void
ParabolicAperture::renderOpenGL()
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
