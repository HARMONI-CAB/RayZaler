//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

#include <MediumBoundaries/PhaseScreen.h>
#include <ReferenceFrame.h>
#include <Surfaces/Circular.h>

using namespace RZ;

PhaseScreenBoundary::PhaseScreenBoundary()
{
  setSurfaceShape(new CircularFlatSurface(m_radius));
}

std::string
PhaseScreenBoundary::name() const
{
  return "PhaseScreenBoundary";
}

void
PhaseScreenBoundary::setRadius(Real R)
{
  m_radius = R;
  surfaceShape<CircularFlatSurface>()->setRadius(R);
}

void
PhaseScreenBoundary::setCoef(unsigned int ansi, Real value)
{
  if (ansi >= m_poly.size()) {
    size_t oldSize = m_poly.size();
    m_poly.resize(ansi + 1);
    m_coef.resize(ansi + 1);
    m_nonz.resize(ansi + 1);

    while (oldSize <= ansi) {
      m_poly[oldSize] = Zernike(oldSize);
      m_coef[oldSize] = 0.;
      m_nonz[oldSize] = false;
      ++oldSize;
    }
  }

  m_coef[ansi] = value;
  m_nonz[ansi] = !isZero(value, 1e-15);

  m_firstNz = -1;
  m_lastNz  = -1;

  for (auto i = 0; i < m_poly.size(); ++i) {
    if (m_nonz[i]) {
      if (m_firstNz == -1)
        m_firstNz = i;
      m_lastNz = i + 1;
    }
  }
}

void
PhaseScreenBoundary::setRefractiveIndex(Real in, Real out)
{
  m_muIn    = in;
  m_muOut   = out;
  m_IOratio = in / out;
}

Real
PhaseScreenBoundary::Z(Real x, Real y) const
{
  Real val = 0;

  for (int i = m_firstNz; i < m_lastNz; ++i)
    if (m_nonz[i])
      val += m_coef[i] * m_poly[i](x, y);

  return val;
}

Real
PhaseScreenBoundary::dZdx(Real x, Real y) const
{
  Real val = 0;

  for (int i = m_firstNz; i < m_lastNz; ++i)
    if (m_nonz[i])
      val += m_coef[i] * m_poly[i].gradX(x, y);

  return val;
}

Real
PhaseScreenBoundary::dZdy(Real x, Real y) const
{
  Real val = 0;

  for (int i = m_firstNz; i < m_lastNz; ++i)
    if (m_nonz[i])
      val += m_coef[i] * m_poly[i].gradY(x, y);

  return val;
}

void
PhaseScreenBoundary::process(RayBeam &beam, const ReferenceFrame *plane) const
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

      Real x  = coordX * Rinv;
      Real y  = coordY * Rinv;
      Real dt = Z(x, y);
      
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

      beam.lengths[i]       += dt;
      beam.cumOptLengths[i] += beam.n * dt;

      // And apply Snell again
      u  = -tiltNormal.cross(nXu) - tiltNormal * sqrt(1 - nXu * nXu);
      u.copyToArray(beam.directions + 3 * i);
    } else {
      // Outside aperture
      beam.prune(i);
    }
  }
}
