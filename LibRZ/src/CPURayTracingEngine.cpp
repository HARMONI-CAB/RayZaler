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

using namespace RZ;

CPURayTracingEngine::CPURayTracingEngine() : RayTracingEngine()
{

}

void
CPURayTracingEngine::cast(Point3 const &center,  Vec3 const &normal, bool reversible)
{
  RayBeam *beam = this->beam();
  Real numDot, demDot, t;
  uint64_t i, N = 3 * beam->count, r = 0;
  uint64_t p = 0;
  RayTracingProcessListener *listenerObj = listener();
  
  numDot = demDot = 0;

  for (i = 0; i < N; ++i) {
    // Center of the surface, w.r.t beam origin 
    beam->destinations[i] = center.coords[p] - beam->origins[i];

    // Scalar products: <n, dest> and <n, d>. Needed for plane-rect 
    // intersection.
    numDot += normal.coords[p] * beam->destinations[i];
    demDot += normal.coords[p] * beam->directions[i];

    if (++p == 3) {
      p = 0;
      t = numDot / demDot;

      if (!reversible && t < 0) {
        beam->prune(r);
      }

      numDot = demDot = 0;
      
      if (t < 0) {
        t = -t;
        /*beam->directions[i - 2] = -beam->directions[i - 2];
        beam->directions[i - 1] = -beam->directions[i - 1];
        beam->directions[i - 0] = -beam->directions[i - 0];*/
      }

      beam->destinations[i - 2] = beam->origins[i - 2] + t * beam->directions[i - 2];
      beam->destinations[i - 1] = beam->origins[i - 1] + t * beam->directions[i - 1];
      beam->destinations[i - 0] = beam->origins[i - 0] + t * beam->directions[i - 0];
      beam->lengths[r]          = t;
      beam->cumOptLengths[r]   += beam->n * t;
      
      ++r;
    }

    if (cancelled())
      break;

    if ((i & 0x3ff) == 0)
      rayProgress(i, N);
  }

  rayProgress(N, N);
}
