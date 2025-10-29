//
//  Copyright (c) 2025 Gonzalo José Carracedo Carballal
//  Copyright (c) 2025 Pablo Álvarez Martín
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

#ifndef _LENS_HELPERS_H
#define _LENS_HELPERS_H

#include <Vector.h>

  namespace RZ {
  struct LensSurfaceProperties 
  {
    Real Rc, Rc2;
    Real sigma;
    bool convex;
    Real displacement;
    
    inline void
    setProperties(Real rCurv, Real K, Real R2) {
      
      Rc = fabs(rCurv);
      Rc2 = Rc * Rc;
      convex = rCurv > 0;
      sigma = convex ? 1 : -1;

      if (isZero(K + 1))
        displacement = .5 * R2 / rCurv;
      else
        displacement = (Rc - sqrt(Rc2 - (K + 1) * R2)) / (K + 1);
    }
  };
  
  static inline Real
  edgeToThickness(Real edgeThickness, const Real &sigma1, const Real &displacement1, const Real &sigma2, const Real &displacement2)
  {
    return edgeThickness + sigma1 * displacement1 + sigma2 * displacement2;
  }
  
  static inline Real
  thicknessToEdge(Real thickness, const Real &sigma1, const Real &displacement1, const Real &sigma2, const Real &displacement2)
  {
    return thickness - sigma1 * displacement1 - sigma2 * displacement2;
  }
  
  static inline bool
  adjustThickness(Real &edge, Real &thickness, bool fromEdge, Real sigma1, Real displacement1, Real sigma2, Real displacement2)
  {
    if (fromEdge) {
      thickness = edgeToThickness(edge, sigma1, displacement1, sigma2, displacement2);
      if (thickness < 0.0) {
        thickness = 0.0;
        edge = thicknessToEdge(thickness, sigma1, displacement1, sigma2, displacement2);
        return false;
      } else {
        return true;
      }
    } else {
      edge = thicknessToEdge(thickness, sigma1, displacement1, sigma2, displacement2);
      if (edge < 0.0) {
        edge = 0.0;
        thickness = edgeToThickness(edge, sigma1, displacement1, sigma2, displacement2);
        return false;
      } else {
        return true;
      }
    }
  }
}

#endif // _LENS_HELPERS_H

