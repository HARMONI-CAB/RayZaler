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

#ifndef _SURFACES_RECTANGULAR_H
#define _SURFACES_RECTANGULAR_H

#include <SurfaceShape.h>

namespace RZ {
  class RectangularFlatSurface : public SurfaceShape {
    Real m_width             = 100e-3;
    Real m_height            = 100e-3;
    void updateEdges();
    std::vector<std::vector<Real>> m_edges;
    
  public:
    inline Real
    width() const
    {
      return m_width;
    }

    inline Real
    height() const
    {
      return m_height;
    }
    
    RectangularFlatSurface();
    virtual ~RectangularFlatSurface();

    void setWidth(Real);
    void setHeight(Real);
    virtual std::vector<std::vector<Real>> const &edges() const override;

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

#endif // _SURFACES_RECTANGULAR_H
