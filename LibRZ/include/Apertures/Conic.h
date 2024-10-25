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

#ifndef _APERTURES_CONIC_H
#define _APERTURES_CONIC_H

#include <GenericAperture.h>
#include <GLHelpers.h>

namespace RZ {
  class ConicAperture : public GenericAperture {
    Real m_radius = 1;
    Real m_radius2 = 1;
    Real m_rCurv   = 2;
    Real m_rCurv2  = 2;
    Real m_rHole   = 0;
    Real m_K = 0;
    
    Real m_x0 = 0;
    Real m_y0 = 0;

    Real m_ux = 1;
    Real m_uy = 0;
    Real m_depth = 1;

    bool m_parabola = false;
    bool m_convex   = false;

    std::vector<GLfloat> m_vertices;
    std::vector<GLfloat> m_holeVertices;
    std::vector<GLfloat> m_axes;

    void recalcGLConic();
    void recalcGLParabolic();
    void recalcGL();
    void recalcDistribution();

  public:
    ConicAperture(Real radius, Real RCurv, Real K);

    void setRadius(Real);
    void setConicConstant(Real);
    void setCurvatureRadius(Real);
    void setCenterOffset(Real, Real);
    void setHoleRadius(Real);
    void setConvex(bool);
    
    virtual bool intercept(
      Vec3 &hit,
      Vec3 &normal,
      Real &tIgnore,
      Vec3 const &origin) const override;
    
    virtual Real area() const override;
    
    virtual void generatePoints(
        const ReferenceFrame *,
        Real *pointArr,
        Real *normals,
        unsigned int N) override;

    virtual void renderOpenGL() override;
  };
}

#endif // _APERTURES_CONIC_H
