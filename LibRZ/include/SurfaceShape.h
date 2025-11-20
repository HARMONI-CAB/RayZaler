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
#include <Logger.h>

#define GENERIC_APERTURE_NUM_SEGMENTS     36
#define GENERIC_APERTURE_NUM_GRIDLINES    13

namespace RZ {

  enum ApertureType{
    Elliptical,
    Rectangular,
  };

  class SurfaceShape {
      ExprRandomState                m_state;
      std::vector<std::vector<Real>> m_emptyEdges;
      bool                           m_complementary = false;
      int                            m_apertureType = ApertureType::Elliptical;
      Real                           m_apertureHeight;
      Real                           m_apertureWidth;
      Real                           m_invApertureHeight2;
      Real                           m_invApertureWidth2;

    public:

      virtual ~SurfaceShape();

      inline void
      setApertureType(ApertureType shape) {
        m_apertureType = shape;
      }

      inline void
      setApertureHeight(Real height) {
        m_apertureHeight = height;
        m_invApertureHeight2 = 1 / (m_apertureHeight * m_apertureHeight);
      }

      inline void
      setApertureWidth(Real width) {
        m_apertureWidth = width;
        m_invApertureWidth2 = 1 / (m_apertureWidth * m_apertureWidth);
      }

      inline bool
      isWithinAperture(Real x, Real y) const { // ver lo de m_x0 y m_y0, pero si lo hago así en realidad puedo usar los x e y que se definen en conic.h en los que se restan ya estas cantidades
        if (m_apertureType == Elliptical) {
          return x * x * m_invApertureWidth2 + y * y * m_invApertureHeight2 <= 1.0;
        } else if (m_apertureType == Rectangular) {
          return x * x <= m_apertureWidth * m_apertureWidth && y * y <= m_apertureHeight * m_apertureHeight;
        } else {
          RZError("Unsupported aperture type %d\n", m_apertureType);
        }

        return false;
      }
      
      inline ExprRandomState &
      randState()
      {
        return m_state;
      }
      
      inline void
      setComplementary(bool comp) {
        m_complementary = comp;
      }

      inline bool
      complementary() const {
        return m_complementary;
      }

      inline bool
      intercept(Vec3 &hit) const
      {
        Vec3 ignore;
        Real tIgnore = 0;
        return intercept(hit, ignore, tIgnore, Vec3::zero(), hit);
      }

      virtual Real area() const = 0;
      virtual std::string name() const = 0;
      virtual bool intercept(
        Vec3 &hit,
        Vec3 &normal,
        Real &dT,
        Vec3 const &origin,
        Vec3 const &direction) const = 0;
      
      virtual void generatePoints(
        const ReferenceFrame *,
        Real *pointArr,
        Real *normals,
        unsigned int N) = 0;

      virtual std::vector<std::vector<Real>> const &edges() const;
      virtual void renderOpenGL();
      virtual void renderOpenGLExtra();
      
  };
}

#endif // _GENERIC_APERTURE_H
