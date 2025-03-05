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

#include <CPURayTracingEngine.h>
#include <MediumBoundary.h>
#include <OpticalElement.h>

using namespace RZ;

// In a Ray Tracing engine, every call to cast and transmit assumes that
// the rays are in the reference frame of the optical surface

CPURayTracingEngine::CPURayTracingEngine() : RayTracingEngine()
{

}

void
CPURayTracingEngine::cast(const OpticalSurface *surface, RayBeam *beam)
{
  uint64_t count = beam->count;
  auto prevBeam = this->beam();
  bool nonSeq = prevBeam->nonSeq;

  // Two cases here:
  //  - The previous beam is sequential, trust the caller.
  //  - The previoys beam is non-sequential. Some rays may be aimed to their
  //    departure surface. Avoid those.

  if (!nonSeq) {
    surface->boundary->cast(RayBeamSlice(beam));
  } else {
    beam->walk(
      const_cast<OpticalSurface *>(surface),
      [&] (OpticalSurface *surf, RayBeamSlice const &slice) {
        surface->boundary->cast(slice);
      },
      [&] (OpticalSurface *surf, RayBeam const *, uint64_t i) {
        return prevBeam->surfaces[i] != surface;
      });
  }

  rayProgress(count, count);
}

void
CPURayTracingEngine::transmit(const OpticalSurface *surface, RayBeam *beam)
{
  beam->walk(
    const_cast<OpticalSurface *>(surface),
    [&] (OpticalSurface *surf, RayBeamSlice const &slice) {
      surf->boundary->transmit(slice);
    }
  );
}
