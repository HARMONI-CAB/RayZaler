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
