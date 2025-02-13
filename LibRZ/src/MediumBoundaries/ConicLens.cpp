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

#include <MediumBoundaries/ConicLens.h>
#include <Surfaces/Conic.h>
#include <ReferenceFrame.h>

using namespace RZ;

ConicLensBoundary::ConicLensBoundary()
{
  setSurfaceShape(new ConicSurface(m_radius, m_rCurv, m_K));
}

std::string
ConicLensBoundary::name() const
{
  return "ConicLensBoundary";
}

void
ConicLensBoundary::setRadius(Real R)
{
  m_radius = R;
  surfaceShape<ConicSurface>()->setRadius(R);
}

void
ConicLensBoundary::setCurvatureRadius(Real Rc)
{
  m_rCurv = Rc;
  surfaceShape<ConicSurface>()->setCurvatureRadius(Rc);
}


void
ConicLensBoundary::setConicConstant(Real K)
{
  m_K = K;
  surfaceShape<ConicSurface>()->setConicConstant(K);
}

void
ConicLensBoundary::setCenterOffset(Real x, Real y)
{
  m_x0 = x;
  m_y0 = y;

  surfaceShape<ConicSurface>()->setCenterOffset(x, y);
}

void
ConicLensBoundary::setRefractiveIndex(Real in, Real out)
{
  m_muIn    = in;
  m_muOut   = out;
  m_IOratio = in / out;
}

void
ConicLensBoundary::setConvex(bool convex)
{
  if (convex != m_convex) {
    m_convex = convex;
    surfaceShape<ConicSurface>()->setConvex(convex);
  }
}

void
ConicLensBoundary::transfer(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;

  for (i = 0; i < count; ++i) {
    if (!beam.hasRay(i))
      continue;
    
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));
    Vec3 orig   = plane->toRelative(Vec3(beam.origins + 3 * i));
    Vec3 normal;
    Real dt;

    if (surfaceShape()->intercept(coord, normal, dt, orig)) {
      beam.lengths[i]       += dt;
      beam.cumOptLengths[i] += beam.n * dt;
      plane->fromRelative(coord).copyToArray(beam.destinations + 3 * i);
      beam.interceptDone(i);

      snell(
        Vec3(beam.directions + 3 * i), 
        plane->fromRelativeVec(normal),
        m_IOratio).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside lens
      beam.prune(i);
    }
  }

  // Yes, we specify the material these rays had to traverse
  beam.n = m_muIn;
}
