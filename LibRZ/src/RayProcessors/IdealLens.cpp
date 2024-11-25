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

#include <RayProcessors/IdealLens.h>
#include <Surfaces/Circular.h>
#include <ReferenceFrame.h>

using namespace RZ;

IdealLensProcessor::IdealLensProcessor()
{
  setSurfaceShape(new CircularFlatSurface(m_radius));
}

std::string
IdealLensProcessor::name() const
{
  return "IdealLens";
}

void
IdealLensProcessor::setRadius(Real R)
{
  m_radius = R;
  surfaceShape<CircularFlatSurface>()->setRadius(R);
}

void
IdealLensProcessor::setFocalLength(Real fLen)
{
  m_fLen = fLen;
}

void
IdealLensProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;

  for (i = 0; i < count; ++i) {
    if (!beam.hasRay(i))
      continue;
    
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));
    
    if (surfaceShape()->intercept(coord)) {
      beam.interceptDone(i);

      Vec3 inDir  = plane->toRelativeVec((beam.directions + 3 * i));

      Real tanRho = sqrt(1 - inDir.x * inDir.x - inDir.y * inDir.y);
      Real tanX   = inDir.x / tanRho;
      Real tanY   = inDir.y / tanRho;

      Vec3 dest   = Vec3(m_fLen * tanX, m_fLen * tanY, -m_fLen);
      Vec3 outDir = (dest - coord).normalized();

      plane->fromRelativeVec(outDir).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside window
      beam.prune(i);
    }
  }
}
