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

#ifndef _EM_FIELDS_EM_ISO_ISO_SOLVER_H
#define _EM_FIELDS_EM_ISO_ISO_SOLVER_H

#include "EMInterfaceSolver.h"

namespace RZ {
  class EMIsoIsoSolver : public EMInterfaceSolver {
    protected:
      virtual uint8_t secondaryBeamCount() const override;
      virtual    void transmit()           override;
    
    public:
      EMIsoIsoSolver(const EMMedium *m1, const EMMedium *m2);
      virtual ~EMIsoIsoSolver() override;
  };
}

#endif // _EM_FIELDS_EM_ISO_ISO_SOLVER_H
