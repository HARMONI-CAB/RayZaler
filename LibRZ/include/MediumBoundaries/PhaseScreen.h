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

#ifndef _RAY_PROCESSORS_PHASE_SCREEN_H
#define _RAY_PROCESSORS_PHASE_SCREEN_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class PhaseScreenBoundary : public MediumBoundary {

    public:
      PhaseScreenBoundary();

      Real Z(Real x, Real y) const;
      Real coef(unsigned int ansi) const;
      void setRadius(Real);
      void setCoef(unsigned int ansi, Real value);
      void setRefractiveIndex(Real, Real);
      virtual std::string name() const;
  };
}

#endif // _RAY_PROCESSORS_PHASE_SCREEN_H
