#include <Apertures/Parabolic.h>

using namespace RZ;

ParabolicAperture::ParabolicAperture(Real R, Real fLength)
{
  setRadius(R);
  setFocalLength(fLength);
}


void
ParabolicAperture::setRadius(Real R)
{
  m_radius2 = R * R;
}

void
ParabolicAperture::setFocalLength(Real fLength)
{
  m_flength = fLength;
}

bool
ParabolicAperture::intercept(
  Vec3 &intercept,
  Vec3 &normal,
  Vec3 const &Ot) const
{
  Vec3 ut = (intercept - Ot).normalized();

  if (Ot.z < 0)
    return false;

  if (intercept.x * intercept.x + intercept.y * intercept.y < m_radius2) {
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

    intercept = Ot + t * ut;
    normal = Vec3(
      0.5 * intercept.x / m_flength,
      0.5 * intercept.y / m_flength,
      1.0).normalized();

    return true;
  }

  return false;
}

void
ParabolicAperture::generatePoints(
    const ReferenceFrame *,
    Real *pointArr,
    unsigned int N)
{
  
}
