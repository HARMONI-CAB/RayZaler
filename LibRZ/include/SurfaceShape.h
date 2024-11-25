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

#ifndef _GENERIC_APERTURE_H
#define _GENERIC_APERTURE_H

#include "Vector.h"
#include "ReferenceFrame.h"
#include <Random.h>
#include <vector>

#define GENERIC_APERTURE_NUM_SEGMENTS     36
#define GENERIC_APERTURE_NUM_GRIDLINES    13

namespace RZ {
  class SurfaceShape {
      ExprRandomState                m_state;
      std::vector<std::vector<Real>> m_emptyEdges;

    public:
      virtual ~SurfaceShape();
      
      inline ExprRandomState &randState()
      {
        return m_state;
      }
      
      inline bool
      intercept(Vec3 &hit) const
      {
        Vec3 ignore;
        Real tIgnore = 0;
        return intercept(hit, ignore, tIgnore, Vec3::zero());
      }

      virtual Real area() const = 0;

      virtual bool intercept(
        Vec3 &hit,
        Vec3 &normal,
        Real &tIgnore,
        Vec3 const &origin) const = 0;
      
      virtual void generatePoints(
        const ReferenceFrame *,
        Real *pointArr,
        Real *normals,
        unsigned int N) = 0;

      virtual std::vector<std::vector<Real>> const &edges() const;
      virtual void renderOpenGL();
      
  };
}

#endif // _GENERIC_APERTURE_H
