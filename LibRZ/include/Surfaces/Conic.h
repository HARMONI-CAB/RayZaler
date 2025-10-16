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

#ifndef _SURFACES_CONIC_H
#define _SURFACES_CONIC_H

#include <SurfaceShape.h>
#include <GLHelpers.h>
#include <Logger.h>

namespace RZ {
  class ConicSurface : public SurfaceShape {
    Real m_radius  = 1;
    Real m_radius2 = 1;
    Real m_rCurv   = 2;
    Real m_rCurv2  = 2;
    Real m_rHole   = 0;
    Real m_rHole2  = 0;
    Real m_K = 0;
    
    Real m_x0 = 0;
    Real m_y0 = 0;

    Real m_ux = 1;
    Real m_uy = 0;
    Real m_depth = 1;

    bool m_parabola = false;
    bool m_convex   = false;
    bool m_dirty    = false;
    
    std::vector<GLfloat> m_vertices;
    std::vector<GLfloat> m_holeVertices;
    std::vector<GLfloat> m_axes;

    std::vector<GLfloat> m_selectedAxes;
    std::vector<GLfloat> m_selectedAxesClosed;
    std::vector<GLfloat> m_selectedEquator;

    std::vector<std::vector<Real>> m_edges;
    
    void recalcSelectionGL();
    void recalcGL();
    void recalcDistribution();

    template<class T> void generateConicCircle(
      T &dest,
      Real r,
      Real x0 = 0, Real y0 = 0,
      Real sign = 1,
      unsigned int segments = GENERIC_APERTURE_NUM_SEGMENTS);
    
    void generateConicSectionVertices(
      std::vector<GLfloat> &dest,
      Real r0,
      Real rn,
      Real x0, Real y0,
      Real ux, Real uy,
      Real sign = 1,
      unsigned int segments = GENERIC_APERTURE_NUM_SEGMENTS);

  public:
    static inline Real
    RmaxNonParNonPar(Real Sp, Real Rcp, Real Kp, Real dp, Real Sn, Real Rcn, Real Kn, Real dn, Real t)
    {
      Real Rmax = 0;
      Real Rc2p = Rcp * Rcp;
      Real Rc2n = Rcn * Rcn;

      Real Ap = Sp * (dp * (Kp + 1) - Rcp) / (Kp + 1);
      Real Bp = Sp * Rcp / (Kp + 1);
      Real Cp = -(Kp + 1) / Rc2p;

      Real An = Sn * (dn * (Kn + 1) - Rcn) / (Kn + 1);
      Real Bn = Sn * Rcn / (Kn + 1);
      Real Cn = -(Kn + 1) / Rc2n;

      Real Bp2 = Bp * Bp;
      Real Cp2 = Cp * Cp;
      Real Bn2 = Bn * Bn;
      Real Cn2 = Cn * Cn;

      Real Bp4 = Bp2 * Bp2;
      Real Bn4 = Bn2 * Bn2;

      Real D = Ap - An + t;
      Real D2 = D * D;
      Real D4 = D2 * D2;

      Real a = Bp4 * Cp2 + Bn4 * Cn2 - 2 * Bp2 * Bn2 * Cp * Cn;
      Real b = 2 * (Bp4 * Cp + Bn4 * Cn - Bp2 * Bn2 * (Cp + Cn) - Bp2 * D2 * Cp - Bn2 * D2 * Cn);
      Real c = Bp4 + Bn4 + D4 - 2 * (Bp2 * Bn2 + Bp2 * D2 + Bn2 * D2);

      Real eps = 1e-12;
      if (!isZero(a, eps)) {
        Real disc = b * b - 4 * a * c;
        if (disc < 0)
        {
          RZWarning("There are no real solutions.\n");
          Rmax = NAN;
        }
        Real y1 = (-b + sqrt(disc)) / (2 * a);
        Real y2 = (-b - sqrt(disc)) / (2 * a);
        
        Real r1 = sqrt(y1);
        Real r2 = sqrt(y2);
        
        Real eq1 = Ap + Bp * sqrt(1 + Cp * r1 * r1) - An - Bn * sqrt(1 + Cn * r1 * r1) + t;
        Real eq2 = Ap + Bp * sqrt(1 + Cp * r2 * r2) - An - Bn * sqrt(1 + Cn * r2 * r2) + t;
        
        if (isZero(eq1, eps)) {
          Rmax = sqrt(y1);
        } else if (isZero(eq2, eps)) {
          Rmax = sqrt(y2);
        } else {
          RZWarning("There are no real solutions.\n");
          Rmax = NAN;
        }
      } else {
        Real y = -c / b;
        if (y < 0) {
          RZWarning("There are no real solutions.\n");
          Rmax = NAN;
        }
        Rmax = sqrt(y);
      }

      return Rmax;
    }
    
    static inline Real
    RmaxParNonPar(Real Sp, Real Rcp, Real Kp, Real dp, Real Sn, Real Rcn, Real Kn, Real dn, Real t)
    {
      Real Rmax = 0;
      Real Rc2p = Rcp * Rcp;
      Real Rc2n = Rcn * Rcn;

      Real A = Sp * (dp * (Kp + 1) - Rcp) / (Kp + 1);
      Real B = Sp * Rcp / (Kp + 1);
      Real C = - (Kp + 1) / Rc2p;

      Real X = Sn * dn;
      Real Y = -Sn / (2 * Rcn);

      Real D = X - A - t;
        
      Real B2 = B * B;
      Real Y2 = Y * Y;
      Real D2 = D * D;

      Real a = Y2;
      Real b = 2 * Y * D - B2 * C;
      Real c = D2 - B2;
        
      Real eps = 1e-12;
      if (isZero(a, eps)) {
        Real disc = b * b - 4 * a * c;
        if (disc < 0) {
          RZWarning("There are no real solutions.\n");
          Rmax = NAN;
        }
        Real y1 = (-b + sqrt(disc)) / (2 * a);
        Real y2 = (-b - sqrt(disc)) / (2 * a);
        
        Real r1 = sqrt(y1);
        Real r2 = sqrt(y2);
        
        Real eq1 = X + Y * r1 * r1 - A - B * sqrt(1 + C * r1 * r1) + t;
        Real eq2 = X + Y * r2 * r2 - A - B * sqrt(1 + C * r2 * r2) + t;
        
        if (isZero(eq1, eps)) {
          Rmax = sqrt(y1);
        } else if (isZero(eq2, eps)) {
          Rmax = sqrt(y2);
        } else {
          RZWarning("There are no real solutions.\n");
          Rmax = NAN;
        }
      } else {
        Real y = -c / b;
        if (y < 0) {
          RZWarning("There are no real solutions.\n");
          Rmax = NAN;
        } 
        Rmax = sqrt(y);
      }
      
      return Rmax;
    }
    
    static inline Real
    RmaxParPar(Real Sp, Real Rcp, Real Kp, Real dp, Real Sn, Real Rcn, Real Kn, Real dn, Real t)
    {
      Real Rmax = 0;
      Real Xp = Sp * dp;
      Real Yp = -Sp / (2 * Rcp);
      Real Xn = Sn * dn;
      Real Yn = -Sn / (2 * Rcn);
        
      Rmax = sqrt((Xn - Xp - t) / (Yp - Yn));
      
      return Rmax;
    }

    static inline Real
    Rmax(Real Sp, Real Rcp, Real Kp, Real dp, Real Sn, Real Rcn, Real Kn, Real dn, Real t) {
      Real Rmax;
      bool isParabolicP = isZero(Kp + 1);
      bool isParabolicN = isZero(Kn + 1);
      if (!isParabolicP && !isParabolicN)
        Rmax = RmaxNonParNonPar(Sp, Rcp, Kp, dp, Sn, Rcn, Kn, dn, t);
      else if (!isParabolicP && isParabolicN) 
        Rmax = RmaxParNonPar(Sp, Rcp, Kp, dp, Sn, Rcn, Kn, dn, t);
      else if (isParabolicP && !isParabolicN) 
        Rmax = RmaxParNonPar(Sp, Rcp, Kp, dp, Sn, Rcn, Kn, dn, -t);
      else 
        Rmax = RmaxParPar(Sp, Rcp, Kp, dp, Sn, Rcn, Kn, dn, t);
      return Rmax;
    }
    
    ConicSurface(Real radius, Real RCurv, Real K);
    virtual ~ConicSurface() = default;
    
    void setRadius(Real);
    void setConicConstant(Real);
    void setCurvatureRadius(Real);
    void setCenterOffset(Real, Real);
    void setHoleRadius(Real);
    void setConvex(bool);
    Real z(Real r) const;

    virtual bool intercept(
      Vec3 &hit,
      Vec3 &normal,
      Real &tIgnore,
      Vec3 const &origin,
      Vec3 const &direction) const override;
    
    virtual Real area() const override;
    virtual std::string name() const override;
    
    virtual void generatePoints(
        const ReferenceFrame *,
        Real *pointArr,
        Real *normals,
        unsigned int N) override;

    virtual std::vector<std::vector<Real>> const &edges() const override;
    virtual void renderOpenGL() override;
    virtual void renderOpenGLExtra() override;
  };
}

#endif // _SURFACES_CONIC_H
