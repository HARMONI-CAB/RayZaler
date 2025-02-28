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

#ifndef _SURFACES_CIRCULAR_H
#define _SURFACES_CIRCULAR_H

#include <GLHelpers.h>
#include <SurfaceShape.h>

namespace RZ {
  class CircularFlatSurface : public SurfaceShape {
    Real m_radius;
    Real m_radius2;
    Real m_a  = 1;
    Real m_b  = 1;
    Real m_a2 = 1;
    Real m_b2 = 1;
    bool m_obstruction = false;

    std::vector<GLfloat> m_vertices;
    std::vector<GLfloat> m_grid;
    std::vector<std::vector<Real>> m_edges;

    void recalculate();
    void renderOpenGLAperture();
    void renderOpenGLObstruction();
    
  public:
    CircularFlatSurface(Real radius);
    virtual ~CircularFlatSurface() = default;
    
    virtual std::vector<std::vector<Real>> const &edges() const override;
    void setRadius(Real);
    void setEccentricity(Real);

    // 
    // Calculate radius and eccentricity from width and height. Given that:
    //
    // x^2 / A^2 + y^2 / B^2 = 1
    //
    // With A = aR and B = bR, then:
    //
    // width  = 2A = 2aR
    // height = 2B = 2bR
    //
    // Since we want to make sure that pi R^2 = piAB, then:
    // 
    // pi width * height / 4 = pi R^2 => R = sqrt(width * height) / 2
    // 
    static inline void
    radiusEccentricity(Real &R, Real &e, Real width, Real height)
    {
      Real a2, b2;

      R   = .5 * sqrt(width * height);
      a2  = .5 * width  / R;
      a2 *= a2;
      b2  = .5 * height / R;
      b2 *= b2;

      if (b2 < a2)
        e = +sqrt(1 - b2 * b2);
      else
        e = -sqrt(1 - a2 * a2);
    }

    inline Real
    a() const
    {
      return sqrt(m_a2);
    }

    inline Real
    b() const
    {
      return sqrt(m_b2);
    }

    inline Real
    a2() const
    {
      return m_a2;
    }

    inline Real
    b2() const
    {
      return m_b2;
    }

    void setObstruction(bool);
    
    virtual bool intercept(
      Vec3 &coord,
      Vec3 &n,
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

    virtual void renderOpenGL() override;
  };
}

#endif // _SURFACES_CIRCULAR_H
