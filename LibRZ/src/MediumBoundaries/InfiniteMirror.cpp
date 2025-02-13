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

#include <MediumBoundaries/InfiniteMirror.h>
#include <ReferenceFrame.h>

using namespace RZ;

std::string
InfiniteMirrorBoundary::name() const
{
  return "InfiniteMirrorBoundary";
}

void
InfiniteMirrorBoundary::transfer(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = 3 * beam.count;
  uint64_t i;
  Vec3 normal = plane->eZ();

  for (i = 0; i < count; ++i) {
    beam.interceptDone(i);
    reflection(
        Vec3(beam.directions + 3 * i), 
        normal).copyToArray(beam.directions + 3 * i);
  }
}
