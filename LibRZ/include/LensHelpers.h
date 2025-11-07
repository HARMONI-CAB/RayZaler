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
#include <Surfaces/Conic.h>

namespace RZ {
  struct LensSurfaceProperties 
  {
    Real radius;
    Real Rc, Rc2;
    Real sigma;
    Real Kc;
    bool convex;
    bool parabola;
    Real displacement;
    Real x0, y0;
    Real rho0, rho02;

    inline void
    setProperties(Real rCurv, Real K, Real R, Real cx0, Real cy0)
    {
      Real R2 = R * R;

      radius = R;

      Rc     = fabs(rCurv);
      Rc2    = Rc * Rc;
      convex = rCurv > 0;
      sigma  = convex ? 1 : -1;

      Kc     = K;
      x0     = cx0;
      y0     = cy0;

      rho02  = x0 * x0 + y0 * y0;
      rho0   = sqrt(rho02);

      parabola = isZero(K + 1);

      if (parabola)
        displacement = .5 * R2 / rCurv;
      else
        displacement = (Rc - sqrt(Rc2 - (K + 1) * R2)) / (K + 1);
    }

    inline Real
    zVal(Real rho) const
    {
      Real r = rho - rho0;
      Real r2 = r * r;
      Real K1 = Kc + 1;

      if (parabola)
        return -sigma * (.5 / Rc * r2 - displacement);
      else
        return -sigma * ((Rc - sqrt(Rc2 - K1 * r2)) / K1 - displacement);
    }

    static inline Real
    Rmax(
      LensSurfaceProperties const &surf1,
      LensSurfaceProperties const &surf2,
      Real edgeThickness)
    {
      return RZ::ConicSurface::Rmax(
        surf1.sigma, surf1.Rc, surf1.Kc, surf1.displacement,
        surf2.sigma, surf2.Rc, surf2.Kc, surf2.displacement,
        edgeThickness);
    }

    static inline bool
    zLimits(
      Real &zSupVal,
      Real &zInfVal,
      LensSurfaceProperties const &surf1,
      LensSurfaceProperties const &surf2,
      Real Rmax)
    {
      Real zSup[4];
      Real zInf[4];

      const Real radius = surf1.radius;
      const Real rho    = surf1.rho0;
      
      if (rho > Rmax - radius)
        return false;

      zSup[0] = surf2.zVal(rho);     // z(0,0) -> vertex
      zSup[1] = surf2.zVal(0);       // z(x0, y0)
      zSup[2] = surf2.zVal(+radius); // z(x0-r, y0-r)
      zSup[3] = surf2.zVal(-radius); // z(x0+r, y0+r)
      
      zInf[0] = surf1.zVal(rho);     // z(0,0) -> vertex
      zInf[1] = surf1.zVal(0);       // z(x0, y0)
      zInf[2] = surf1.zVal(+radius); // z(x0-r, y0-r)
      zInf[3] = surf1.zVal(-radius); // z(x0+r, y0+r)

      if (rho < radius) {
        zSupVal += fmin(zSup[0], fmin(zSup[1], fmin(zSup[2], zSup[3])));
        zInfVal += fmax(zInf[0], fmax(zInf[1], fmax(zInf[2], zInf[3])));
      } else {
        zSupVal += fmin(zSup[1], fmin(zSup[2], zSup[3]));
        zInfVal += fmax(zInf[1], fmax(zInf[2], zInf[3]));
      }

      return true;
    }
  };
  
  static inline Real
  edgeToThickness(
    Real edgeThickness,
    const Real &sigma1,
    const Real &displacement1,
    const Real &sigma2,
    const Real &displacement2)
  {
    return edgeThickness + sigma1 * displacement1 - sigma2 * displacement2;
  }
  
  static inline Real
  thicknessToEdge(
    Real thickness,
    const Real &sigma1,
    const Real &displacement1,
    const Real &sigma2,
    const Real &displacement2)
  {
    return thickness - sigma1 * displacement1 + sigma2 * displacement2;
  }
  
  static inline bool
  adjustThickness(
    Real &edge,
    Real &thickness,
    bool fromEdge,
    Real sigma1,
    Real disp1,
    Real sigma2,
    Real disp2)
  {
    if (fromEdge) {
      thickness = edgeToThickness(edge, sigma1, disp1, sigma2, disp2);
      if (thickness < 0.0) {
        thickness = 0.0;
        edge = thicknessToEdge(thickness, sigma1, disp1, sigma2, disp2);
        return false;
      }
    } else {
      edge = thicknessToEdge(thickness, sigma1, disp1, sigma2, disp2);
      if (edge < 0.0) {
        edge = 0.0;
        thickness = edgeToThickness(edge, sigma1, disp1, sigma2, disp2);
        return false;
      }
    }

    return true;
  }
}

#endif // _LENS_HELPERS_H

