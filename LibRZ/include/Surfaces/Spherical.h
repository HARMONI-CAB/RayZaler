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

#ifndef _SURFACES_SPHERICAL_H
#define _SURFACES_SPHERICAL_H

#include <GLHelpers.h>
#include <SurfaceShape.h>

namespace RZ {
  class SphericalSurface : public SurfaceShape {
    Real m_radius;
    Real m_radius2;
    Real m_rCurv  =  1;
    bool m_convex = true;
    Real m_center = .866;
    Real m_K = 1.; // Normalization constant for generatePoints
    Real m_rCurv2 = 1;
    Real m_KRcInv = 1.;
    Real m_x0 = 0;
    Real m_y0 = 0;
    Real m_ux = 1;
    Real m_uy = 0;

    std::vector<GLfloat>           m_vertices;
    std::vector<GLfloat>           m_axes;
    std::vector<std::vector<Real>> m_edges;

    void recalcGL();
    void recalcDistribution();

  public:
    inline Real
    centerOffset() const
    {
      return m_convex ? +m_center : -m_center;
    }

    SphericalSurface(Real radius, Real rCurv);

    void setCenterOffset(Real, Real);
    void setConvex(bool);
    void setRadius(Real);
    void setCurvatureRadius(Real);

    virtual bool intercept(
      Vec3 &hit,
      Vec3 &normal,
      Real &tIgnore,
      Vec3 const &origin) const override;
    
    virtual Real area() const override;
    
    virtual void generatePoints(
        const ReferenceFrame *,
        Real *pointArr,
        Real *normalArr,
        unsigned int N) override;

    virtual void renderOpenGL() override;
  };
}

#endif // _SURFACES_SPHERICAL_H
