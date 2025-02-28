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
ReflectiveEMInterface::transmit(RayBeamSlice const &slice)
{
  blockLight(slice); // Prune rays according to transmission

  auto beam = slice.beam;
  for (auto i = slice.start; i < slice.end; ++i)
    if (mustTransmitRay(slice.beam, i))
      reflection(
        Vec3(beam->directions + 3 * i),
        Vec3(beam->normals    + 3 * i)).copyToArray(beam->directions + 3 * i);
}

ReflectiveEMInterface::~ReflectiveEMInterface()
{

}
