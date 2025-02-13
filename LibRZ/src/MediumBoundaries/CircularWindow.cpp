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

#include <MediumBoundaries/CircularWindow.h>
#include <Surfaces/Circular.h>
#include <ReferenceFrame.h>

using namespace RZ;

CircularWindowBoundary::CircularWindowBoundary()
{
  setSurfaceShape(new CircularFlatSurface(m_radius));
}

std::string
CircularWindowBoundary::name() const
{
  return "CircularWindowBoundary";
}

void
CircularWindowBoundary::setRadius(Real R)
{
  m_radius = R;
  surfaceShape<CircularFlatSurface>()->setRadius(R);
}

void
CircularWindowBoundary::setRefractiveIndex(Real in, Real out)
{
  m_muIn  = in;
  m_muOut = out;
  m_IOratio = in / out;
}

void
CircularWindowBoundary::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;

  for (i = 0; i < count; ++i) {
    if (!beam.hasRay(i))
      continue;
    
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));
    Vec3 orig   = plane->toRelative(Vec3(beam.origins + 3 * i));
    Vec3 normal = Vec3::eZ();

    if (surfaceShape()->intercept(coord)) {
      beam.interceptDone(i);
      
      snell(
        Vec3(beam.directions + 3 * i), 
        plane->fromRelativeVec(normal),
        m_IOratio).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside window
      beam.prune(i);
    }
  }

  // Yes, we specify the material these rays had to traverse
  beam.n = m_muOut;
}
