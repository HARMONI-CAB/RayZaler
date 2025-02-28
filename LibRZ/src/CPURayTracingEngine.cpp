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
  uint64_t i, count = beam->count;
  auto listenerObj = listener();

  // Cast rays to this boundary by means of the CPU
  surface->boundary->cast(*beam);

  for (i = 0; i < count; ++i) {
    if (beam->hasRay(i)) {
      if (beam->isIntercepted(i)) {
        if (beam->lengths[i] < 0)
          beam->prune(i);
      }
    }

    if (cancelled())
      break;

    if ((i & 0x3ff) == 0)
      rayProgress(i, count);
  }

  rayProgress(count, count);
}

void
CPURayTracingEngine::transmit(const OpticalSurface *surface, RayBeam *beam)
{
  beam->walk(
    const_cast<OpticalSurface *>(surface),
    [&] (OpticalSurface *surf, RayBeamSlice const &slice) {
      printf("Transmit slice: %p[%d..%d] -> %s\n", slice.beam, slice.start, slice.end, surf->parent->name().c_str());
      for (auto i = slice.start; i < slice.end; ++i) {
        printf(
          "==> %s: %s (length %g)\n",
          slice.beam->surfaces[i]->parent->name().c_str(),
          Vec3(slice.beam->directions + 3 * i).toString().c_str(),
          slice.beam->lengths[i]);
      }
      
      surf->boundary->transmit(slice);
    }
  );
}
