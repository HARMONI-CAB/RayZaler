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

#include <EMInterfaces/ParaxialZernikeEMInterface.h>
#include <RayTracingEngine.h>

using namespace RZ;

std::string
ParaxialZernikeEMInterface::name() const
{
  return "ParaxialZernikeEMInterface";
}

void
ParaxialZernikeEMInterface::setRadius(Real R)
{
  m_radius = R;
}

void
ParaxialZernikeEMInterface::setRefractiveIndex(Real in, Real out)
{
  m_muIn    = in;
  m_muOut   = out;
  m_IOratio = in / out;
}

void
ParaxialZernikeEMInterface::setCoef(unsigned int ansi, Real value)
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


Real
ParaxialZernikeEMInterface::Z(Real x, Real y) const
{
  Real val = 0;

  for (int i = m_firstNz; i < m_lastNz; ++i)
    if (m_nonz[i])
      val += m_coef[i] * m_poly[i](x, y);

  return val;
}

Real
ParaxialZernikeEMInterface::dZdx(Real x, Real y) const
{
  Real val = 0;

  for (int i = m_firstNz; i < m_lastNz; ++i)
    if (m_nonz[i])
      val += m_coef[i] * m_poly[i].gradX(x, y);

  return val;
}

Real
ParaxialZernikeEMInterface::dZdy(Real x, Real y) const
{
  Real val = 0;

  for (int i = m_firstNz; i < m_lastNz; ++i)
    if (m_nonz[i])
      val += m_coef[i] * m_poly[i].gradY(x, y);

  return val;
}

void
ParaxialZernikeEMInterface::transmit(RayBeam &beam)
{
  uint64_t count = beam.count;
  uint64_t i;
  Real Rinv    = 1. / m_radius;
  Real Rsq     = m_radius * m_radius;

  blockLight(beam); // Prune rays according to transmission

  for (i = 0; i < count; ++i) {
    if (beam.hasRay(i) && beam.isIntercepted(i)) {
      Vec3 coord(beam.destinations + 3 * i);

      //  In the capture surface
      if (coord.x * coord.x + coord.y * coord.y < Rsq) {
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

        Real x  = coord.x * Rinv;
        Real y  = coord.y * Rinv;
        Real dt = Z(x, y);
        
        // An infinitesimal variation of this points of x' = x + dx and 
        // y' = y + dy introduces a change in the 3D point in the directions:
        //
        //   Vx = dp/dx(x, y) = (1, 0, dZ(x, y)/dx)^T
        //   Vy = dp/dy(x, y) = (0, 1, dZ(x, y)/dy)^T
        //

        Vec3 Vx = Vec3(1, 0, dZdx(x, y) * Rinv);
        Vec3 Vy = Vec3(0, 1, dZdy(x, y) * Rinv);

        //
        // These two vectors form a triangle with a vertex in (x, y, Z(x, y)). Also,
        // if Z is smooth, these vectors are always linearly independent. 
        // Therefore, we can calculate their cross product Vx x Vy, which
        // happens to have the same direction as the equivalent surface normal.
        //
        
        Vec3 tiltNormal = Vy.cross(Vx).normalized();

        // And apply Snell again
        snell(
          Vec3(beam.directions + 3 * i),
          Vec3(tiltNormal),
          m_IOratio).copyToArray(beam.directions + 3 * i);
      
        // This ray has entered a new medium. Mark accordingly.
        beam.refNdx[i] = m_muOut;
      }
    }
  }
}

ParaxialZernikeEMInterface::~ParaxialZernikeEMInterface()
{

}
