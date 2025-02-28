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

#include <EMInterfaces/ParaxialEMInterface.h>
#include <RayTracingEngine.h>

using namespace RZ;

std::string
ParaxialEMInterface::name() const
{
  return "ParaxialEMInterface";
}

void
ParaxialEMInterface::setFocalLength(Real fLen)
{
  m_fLen = fLen;
}

void
ParaxialEMInterface::transmit(RayBeamSlice const &slice)
{
  blockLight(slice); // Prune rays according to transmission

  auto beam = slice.beam;
  for (auto i = slice.start; i < slice.end; ++i) {
    if (mustTransmitRay(slice.beam, i)) {
      Vec3 coord(beam->destinations + 3 * i);
      Vec3 inDir(beam->directions + 3 * i);

      Real tanRho = sqrt(1 - inDir.x * inDir.x - inDir.y * inDir.y);
      Real tanX   = inDir.x / tanRho;
      Real tanY   = inDir.y / tanRho;

      Vec3 dest(m_fLen * tanX, m_fLen * tanY, -m_fLen);
      (dest - coord).normalized().copyToArray(beam->directions + 3 * i);
    }
  }
}

ParaxialEMInterface::~ParaxialEMInterface()
{

}

