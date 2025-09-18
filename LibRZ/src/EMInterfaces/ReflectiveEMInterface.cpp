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

#include <EMInterfaces/ReflectiveEMInterface.h>
#include <RayTracingEngine.h>

using namespace RZ;

std::string
ReflectiveEMInterface::name() const
{
  return "ReflectiveEMInterface";
}

void
ReflectiveEMInterface::transmit(
  RayBeamSlice const &slice,
  RayBeam *splinterRays)
{
  blockLight(slice); // Prune rays according to transmission

  auto beam = slice.beam;
  for (auto i = slice.start; i < slice.end; ++i)
    if (mustTransmitRay(slice.beam, i)) {
      if (!beam->media[i]->isotropic())
        throw std::runtime_error("Reflection on birefringent media is not currently implemented");
      
      const Vec3 normal(beam->normals + 3 * i);
      const Vec3 dir(beam->directions + 3 * i);

      auto reflected = reflection(dir, normal);

      reflected.copyToArray(beam->directions + 3 * i);
      (beam->media[i]->n * reflected).copyToArray(beam->k + 3 * i);

      if (beam->fields) {
        const Vec3 iix(beam->vDx + 3 * i);
        const Vec3 iiy = dir.cross(iix);
        
        reflection(iix, normal).copyToArray(beam->vDx + 3 * i);

        auto iixn = iix * normal;
        auto iiyn = iiy * normal;

        beam->Dx[i] *= 1 / (2 * iixn * iixn - 1);
        beam->Dy[i] *= 1 / (2 * iiyn * iiyn - 1);
      }
    }
}

ReflectiveEMInterface::~ReflectiveEMInterface()
{

}
