#include <RayProcessors/PhaseScreen.h>
#include <ReferenceFrame.h>

using namespace RZ;

std::string
PhaseScreenProcessor::name() const
{
  return "PhaseScreenProcessor";
}

void
PhaseScreenProcessor::setRadius(Real R)
{
  m_radius = R;
}

void
PhaseScreenProcessor::setCoef(unsigned int ansi, Real value)
{
  if (ansi >= m_poly.size()) {
    size_t oldSize = m_poly.size();
    m_poly.resize(ansi + 1);
    m_coef.resize(ansi + 1);

    while (oldSize <= ansi) {
      m_poly[oldSize] = Zernike(oldSize);
      m_coef[oldSize] = 0.;
      ++oldSize;
    }
  }

  m_coef[ansi] = value;
}

void
PhaseScreenProcessor::setRefractiveIndex(Real in, Real out)
{
  m_muIn    = in;
  m_muOut   = out;
  m_IOratio = in / out;
}

Real
PhaseScreenProcessor::Z(Real x, Real y) const
{
  Real val = 0;
  unsigned count = m_coef.size();

  for (unsigned i = 0; i < count; ++i)
    val += m_coef[i] * m_poly[i](x, y);

  return val;
}

Real
PhaseScreenProcessor::dZdx(Real x, Real y) const
{
  Real val = 0;
  unsigned count = m_coef.size();

  for (unsigned i = 0; i < count; ++i)
    val += m_coef[i] * m_poly[i].gradX(x, y);

  return val;
}

Real
PhaseScreenProcessor::dZdy(Real x, Real y) const
{
  Real val = 0;
  unsigned count = m_coef.size();

  for (unsigned i = 0; i < count; ++i)
    val += m_coef[i] * m_poly[i].gradY(x, y);

  return val;
}

void
PhaseScreenProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Vec3 center  = plane->getCenter();
  Vec3 tX      = plane->eX();
  Vec3 tY      = plane->eY();
  Vec3 normal  = plane->eZ();
  Real Rinv    = 1. / m_radius;
  Real Rsq     = m_radius * m_radius;

  for (i = 0; i < count; ++i) {
    Vec3 coord  = Vec3(beam.destinations + 3 * i) - center;
    Real coordX = coord * tX;
    Real coordY = coord * tY;

    //  In the capture surface
    if (coordX * coordX + coordY * coordY < Rsq) {
      // 
      // In phase screens, we do not adjust an intercept point, but the
      // direction of the outgoing ray. This is done by estimating the
      // gradient of the equivalent height at the interception point.
      //
      // We start by remarking that the Zernike expansion represents the
      // height of the "equivalent" surface. The units of grad(Z) are therefore
      // dimensionless and represent the tangent of the slope at that point,
      // calculated as dz/dx and dz/dy
      // 
      // We can obtain the normal as follows. Consider the points in the 3D
      // surface defined as:
      //
      //   p(x, y) = (x, y, Z(x, y))^T \forall x, y in D
      //
      // With the points x, y normalized by the aperture radius:

      Real x = coordX * Rinv;
      Real y = coordY * Rinv;

      // An infinitesimal variation of this points of x' = x + dx and 
      // y' = y + dy introduces a change in the 3D point in the directions:
      //
      //   Vx = dp/dx(x, y) = (1, 0, dZ(x, y)/dx)^T
      //   Vy = dp/dy(x, y) = (0, 1, dZ(x, y)/dy)^T
      //

      Vec3 Vx = tX + normal * dZdx(x, y) * Rinv;
      Vec3 Vy = tY + normal * dZdy(x, y) * Rinv;

      //
      // These two vectors form a triangle with a vertex in (x, y, Z(x, y)). Also,
      // if Z is smooth, these vectors are always linearly independent. 
      // Therefore, we can calculate their cross product Vx x Vy, which
      // happens to have the same direction as the surface normal.
      //
      
      Vec3 tiltNormal = Vy.cross(Vx).normalized();
      Vec3 u(beam.directions + 3 * i);
      Vec3 nXu    = m_IOratio * tiltNormal.cross(u);

      // And apply Snell again
      u  = -tiltNormal.cross(nXu) - tiltNormal * sqrt(1 - nXu * nXu);
      u.copyToArray(beam.directions + 3 * i);
    } else {
      // Outside aperture
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));
}
