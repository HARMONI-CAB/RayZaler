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

#include <MediumBoundaries/FlatMirror.h>
#include <Surfaces/Circular.h>
#include <ReferenceFrame.h>

using namespace RZ;

FlatMirrorProcessor::FlatMirrorProcessor()
{
  setSurfaceShape(new CircularFlatSurface(m_radius));
}

std::string
FlatMirrorProcessor::name() const
{
  return "FlatMirrorProcessor";
}

void
FlatMirrorProcessor::setRadius(Real R)
{
  surfaceShape<CircularFlatSurface>()->setRadius(R);

  m_radius = R;
}

void
FlatMirrorProcessor::setEccentricity(Real ecc)
{
  surfaceShape<CircularFlatSurface>()->setEccentricity(ecc);

  m_ecc = ecc;
}

void
FlatMirrorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Vec3 normal = plane->eZ();
  
  for (i = 0; i < count; ++i) {
    if (!beam.hasRay(i))
      continue;
    
    // Check intercept
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));

    if (surfaceShape()->intercept(coord)) {
      beam.interceptDone(i);
      reflection(
        Vec3(beam.directions + 3 * i), 
        normal).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside mirror
      beam.prune(i);
    }
  }
}
