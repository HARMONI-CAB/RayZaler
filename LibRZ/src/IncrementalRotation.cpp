#include <IncrementalRotation.h>

using namespace RZ;


IncrementalRotation::IncrementalRotation()
{
  m_k = Vec3(1, 0, 0);
  m_theta = 0;

  m_R = Matrix3::eye();
}

Vec3
IncrementalRotation::S12(Vec3 const &r)
{
  if (releq(r.norm(), M_PI)
   && (isZero(r.x) && isZero(r.y) && r.z < 0)
   || (isZero(r.x) && r.z < 0)
   || r.x < 0)
     return -r;
  
  return r;
}

void
IncrementalRotation::toRodrigues()
{
  Vec3 r = Vec3::zero();
  Vec3 u = Vec3::zero();

  // Obtain Rodrigues vector
  auto A   = .5 * (m_R - m_R.t());
  auto rho = Vec3(A.coef[1][2], A.coef[0][2], A.coef[1][0]);
  auto s   = rho.norm();
  auto c   = .5 * (m_R.tr() - 1);

  if (isZero(s) && releq(c, -1)) {
    auto RIt = (m_R + Matrix3::eye()).t();
    Vec3 v = Vec3::zero();

    for (auto i = 0; i < 3; ++i) {
      if (!RIt.rows[i].isNull()) {
        v = RIt.rows[i];
        break;
      }
    }

    u = v.normalized();
    r = S12(M_PI * u);

    c = 1;
  }

  if (!isZero(s)) {
    m_k     = rho / s;
    m_theta = atan2(s, c);
    m_k.x = -m_k.x;
  }

  m_R = Matrix3::rot(m_k, m_theta);
}

void
IncrementalRotation::rotateRelative(Vec3 const &vec, Real theta)
{  
  auto R = Matrix3::rot(vec, theta);

  m_R = m_R * R;

  toRodrigues();
}

void
IncrementalRotation::rotate(Vec3 const &vec, Real theta)
{  
  auto R = Matrix3::rot(m_R.t() * vec, theta);

  m_R = m_R * R;

  toRodrigues();
}

void
IncrementalRotation::setRotation(Vec3 const &vec, Real theta)
{
  m_R = Matrix3::rot(vec, theta);

  m_k     = vec;
  m_theta = theta;
}

void
IncrementalRotation::setAzEl(Real az, Real el)
{
  m_R = Matrix3::azel(az, el);

  toRodrigues();
}
