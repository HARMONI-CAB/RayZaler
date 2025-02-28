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

#ifndef _RZ_TESTS_COMMON_H
#define _RZ_TESTS_COMMON_H

#include <OpticalElement.h>

namespace RZ {
  struct BeamTestStatistics {
    Real maxRad = 0.0;
    Real rmsRad = 0.0;
    
    Real x0     = 0.0;
    Real y0     = 0.0;

    Real fNum   = std::numeric_limits<Real>::infinity();

    uint64_t intercepted = 0;
    uint64_t vignetted   = 0;
    uint64_t pruned      = 0;

    void computeFromSurface(
      OpticalSurface const *fp,
      Vec3 const &chiefRay = -Vec3::eZ());

    void computeFromRayList(
      std::list<Ray> const &rays,
      Vec3 const &chiefRay = -Vec3::eZ());
  };
}

#endif // _RZ_TESTS_COMMON_H
