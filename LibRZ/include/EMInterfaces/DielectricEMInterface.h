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

#ifndef _EM_INTERFACES_DIELECTRIC_H
#define _EM_INTERFACES_DIELECTRIC_H

#include <EMInterface.h>

namespace RZ {
  enum DielectricInterfaceCase {
    IsoToIso,
    AnisoToIso,
    IsoToAniso,
    AnisoToAniso
  };

  class DielectricEMInterface : public EMInterface {
      Real m_n2   = 1.5;
      Real m_n1   = 1;
      Real m_n1n2 = 1 / 1.5;
      DielectricInterfaceCase m_interfaceCase = IsoToIso;

      bool detectInterfaceCase();

      inline void calcIsoToIsoFields(
        RayBeam *inputBeam,
        uint64_t inputRay,
        RayBeam *splinterBeam,
        uint64_t splinterRay,
        const Vec3 &ui);


    public:
      void setRefractiveIndex(Real , Real);
      
      virtual std::string name() const override;
      virtual void transmit(RayBeamSlice const &beam, RayBeam *) override;
      virtual void setSurroundingMedium(const EMMedium *) override;
      virtual void setMedia(
        const EMMedium *positive = nullptr,
        const EMMedium *negative = nullptr) override;
      virtual ~DielectricEMInterface() override;
  };
}

#endif // _EM_INTERFACES_DIELECTRIC_H
