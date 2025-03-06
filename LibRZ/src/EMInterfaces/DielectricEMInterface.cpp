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

#include <EMInterfaces/DielectricEMInterface.h>
#include <RayTracingEngine.h>

using namespace RZ;

std::string
DielectricEMInterface::name() const
{
  return "DielectricEMInterface";
}

void
DielectricEMInterface::setRefractiveIndex(Real in, Real out)
{
  m_muIn    = in;
  m_muOut   = out;
  m_IOratio = in / out;
}

void
DielectricEMInterface::transmit(RayBeamSlice const &slice)
{
  blockLight(slice); // Prune rays according to transmission

  //
  // TODO: TEST FOR SPECULAR REFLECTION
  //

  auto beam = slice.beam;
  Real rdir = m_IOratio, rinv = 1 / m_IOratio;
  Real nIn  = m_muIn;
  Real nOu  = m_muOut;

  for (auto i = slice.start; i < slice.end; ++i) {
    if (mustTransmitRay(slice.beam, i)) {
      const Vec3 direct(beam->directions + 3 * i);
      const Vec3 normal(beam->normals    + 3 * i);
      
      if (direct * normal < 0) {
        snell(direct, normal, rdir).copyToArray(beam->directions + 3 * i);
        beam->refNdx[i] = nOu;
      } else {
        snell(direct, -normal, rinv).copyToArray(beam->directions + 3 * i);
        beam->refNdx[i] = nIn;
      }
    }
  }
}

DielectricEMInterface::~DielectricEMInterface()
{

}
