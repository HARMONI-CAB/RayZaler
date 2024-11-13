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

#include <RayProcessors/ConicMirror.h>
#include <Apertures/Conic.h>
#include <ReferenceFrame.h>

using namespace RZ;

ConicMirrorProcessor::ConicMirrorProcessor()
{
  defineAperture(new ConicAperture(m_radius, m_rCurv, m_K));
}

std::string
ConicMirrorProcessor::name() const
{
  return "ConicMirror";
}

void
ConicMirrorProcessor::setRadius(Real R)
{
  m_radius = R;
  aperture<ConicAperture>()->setRadius(R);
}

void
ConicMirrorProcessor::setCurvatureRadius(Real Rc)
{
  m_rCurv = Rc;
  aperture<ConicAperture>()->setCurvatureRadius(Rc);
}

void
ConicMirrorProcessor::setHoleRadius(Real Rh)
{
  m_rHole  = Rh;
  m_rHole2 = Rh * Rh;
  aperture<ConicAperture>()->setHoleRadius(Rh);
}


void
ConicMirrorProcessor::setConicConstant(Real K)
{
  m_K = K;
  aperture<ConicAperture>()->setConicConstant(K);
}

void
ConicMirrorProcessor::setCenterOffset(Real x, Real y)
{
  m_x0 = x;
  m_y0 = y;

  aperture<ConicAperture>()->setCenterOffset(x, y);
}

void
ConicMirrorProcessor::setConvex(bool convex)
{
  if (convex != m_convex) {
    m_convex = convex;
    aperture<ConicAperture>()->setConvex(convex);
  }
}

void
ConicMirrorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
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
 
    if (aperture()->intercept(coord, normal, dt, origin)) {
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

