//
//  Copyright (c) 2025 Gonzalo José Carracedo Carballal
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

#ifndef _EM_FIELDS_EM_INTERFACE_SOLVER_H
#define _EM_FIELDS_EM_INTERFACE_SOLVER_H

#include <EMInterface.h>
#include <Vector.h>

namespace RZ {
  class EMSolver;

  class EMInterfaceSolver {
    protected:
      const EMMedium       *m_m1              = nullptr;
      const EMMedium       *m_m2              = nullptr;
      const RayBeamSlice   *m_currentSlice    = nullptr;
      EMSolver             *m_solver;
      bool                  m_secondaryRays   = false;
      bool                  m_calculateFields = false;
      RayBeam              *m_mainBeam        = nullptr;
      RayBeam              *m_splinterBeam    = nullptr;
      uint8_t               m_sCount          = 0;

      EMInterfaceSolver(
        const EMMedium *m1,
        const EMMedium *m2,
        const ReferenceFrame *frame = nullptr);

      virtual uint8_t secondaryBeamCount() const = 0;
      
  public:
      inline const EMMedium *pMedium() const { return m_m1; }
      inline const EMMedium *nMedium() const { return m_m2; }
      inline       EMSolver *solver()  const { return m_solver; }
      static EMInterfaceSolver *make(
        const EMMedium *m1,
        const EMMedium *m2,
        const ReferenceFrame *frame);

      void setBeam(RayBeamSlice const &slice, RayBeam *splinterBeam);
      void setMedia(const EMMedium *m1, const EMMedium *m2);
      
      virtual void transmit() = 0;

      virtual ~EMInterfaceSolver() = default;
  };
}

#endif // _EM_FIELDS_EM_INTERFACE_SOLVER_H
