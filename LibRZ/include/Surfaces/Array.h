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

#ifndef _SURFACES_ARRAY_H
#define _SURFACES_ARRAY_H

#include <SurfaceShape.h>

namespace RZ {
  class SurfaceArray : public SurfaceShape {
    SurfaceShape *m_subAperture = nullptr;
    Real m_width             = 100e-3;
    Real m_height            = 100e-3;
    unsigned int m_rows      = 10;
    unsigned int m_cols      = 10;
    Real m_subApertureWidth  = 10e-3;
    Real m_subApertureHeight = 10e-3;
    std::vector<std::vector<Real>> m_edges;
    
    void recalculateDimensions();

  public:
    inline SurfaceShape *subAperture() const
    {
      return m_subAperture;
    }

    template <class T>
    inline T *subAperture()
    {
      return static_cast<T *>(subAperture());
    }
    
    template <class T>
    inline T const *subAperture() const
    {
      return static_cast<const T *>(subAperture());
    }

    inline Real
    subApertureWidth() const
    {
      return m_subApertureWidth;
    }

    inline Real
    subApertureHeight() const
    {
      return m_subApertureHeight;
    }

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
    
    virtual std::vector<std::vector<Real>> const &edges() const override;

    SurfaceArray(SurfaceShape *);
    virtual ~SurfaceArray();

    void setWidth(Real);
    void setHeight(Real);
    void setCols(unsigned);
    void setRows(unsigned);

    virtual bool intercept(
      Vec3 &coord,
      Vec3 &n,
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

#endif // _SURFACES_ARRAY_H
