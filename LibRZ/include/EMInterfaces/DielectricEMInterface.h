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
  class EMInterfaceSolver;
  class DielectricEMInterface : public EMInterface {
      EMInterfaceSolver *m_ifaceSolver = nullptr;

      void initInterfaceSolver();

    public:

      virtual std::string name() const override;
      virtual void transmit(RayBeamSlice const &beam, RayBeam *) override;
      virtual void setParentFrame(const ReferenceFrame *) override;
      virtual void setSurroundingMedium(const EMMedium *) override;
      virtual void setMedia(
        const EMMedium *positive = nullptr,
        const EMMedium *negative = nullptr) override;
      virtual ~DielectricEMInterface() override;
  };
}

#endif // _EM_INTERFACES_DIELECTRIC_H
