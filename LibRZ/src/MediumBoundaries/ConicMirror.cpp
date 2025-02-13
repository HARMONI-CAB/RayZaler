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

#include <MediumBoundaries/ConicMirror.h>
#include <Surfaces/Conic.h>
#include <ReferenceFrame.h>

using namespace RZ;

ConicMirrorBoundary::ConicMirrorBoundary()
{
  setSurfaceShape(new ConicSurface(m_radius, m_rCurv, m_K));
}

std::string
ConicMirrorBoundary::name() const
{
  return "ConicMirrorBoundary";
}

void
ConicMirrorBoundary::setRadius(Real R)
{
  m_radius = R;
  surfaceShape<ConicSurface>()->setRadius(R);
}

void
ConicMirrorBoundary::setCurvatureRadius(Real Rc)
{
  m_rCurv = Rc;
  surfaceShape<ConicSurface>()->setCurvatureRadius(Rc);
}

void
ConicMirrorBoundary::setHoleRadius(Real Rh)
{
  m_rHole  = Rh;
  m_rHole2 = Rh * Rh;
  surfaceShape<ConicSurface>()->setHoleRadius(Rh);
}


void
ConicMirrorBoundary::setConicConstant(Real K)
{
  m_K = K;
  surfaceShape<ConicSurface>()->setConicConstant(K);
}

void
ConicMirrorBoundary::setCenterOffset(Real x, Real y)
{
  m_x0 = x;
  m_y0 = y;

  surfaceShape<ConicSurface>()->setCenterOffset(x, y);
}

void
ConicMirrorBoundary::setConvex(bool convex)
{
  if (convex != m_convex) {
    m_convex = convex;
    surfaceShape<ConicSurface>()->setConvex(convex);
  }
}

void
ConicMirrorBoundary::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;

  bool haveHole = !isZero(m_rHole2);
  bool hit = true;
  for (auto i = 0; i < count; ++i) {
    if (!beam.hasRay(i))
      continue;

    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));
    Vec3 origin = plane->toRelative(Vec3(beam.origins + 3 * i));
    Vec3 normal;
    Real dt;
 
    if (surfaceShape()->intercept(coord, normal, dt, origin)) {
      beam.lengths[i]       += dt;
      beam.cumOptLengths[i] += beam.n * dt;

      plane->fromRelative(coord).copyToArray(beam.destinations + 3 * i);
      beam.interceptDone(i);

      if (haveHole) {
        Real x    = coord.x - m_x0;
        Real y    = coord.y - m_y0;
        Real rho2 = x * x + y * y;
        hit = rho2 > m_rHole2;
      }

      if (hit) {
        auto vec = reflection(
          Vec3(beam.directions + 3 * i), 
          plane->fromRelativeVec(normal));
        vec.copyToArray(beam.directions + 3 * i);
      }
    } else {
      // Outside mirror
      beam.prune(i);
    }
  }
}

