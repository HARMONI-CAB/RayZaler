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

#ifndef _INCREMENTAL_ROTATION_H
#define _INCREMENTAL_ROTATION_H

#include "Matrix.h"

namespace RZ {
  class IncrementalRotation {
    Matrix3 m_R, m_Q, m_sinK, m_oneminuscosK2;
    Vec3 m_k;
    Real m_theta;

    static Vec3 S12(Vec3 const &);
    void toRodrigues();

  public:
    IncrementalRotation();

    inline Vec3
    k() const
    {
      return m_k;
    }

    inline Real
    theta() const
    {
      return m_theta;
    }

    inline Matrix3
    matrix() const
    {
      return m_R;
    }

    void rotateRelative(Vec3 const &vec, Real theta);
    void rotate(Vec3 const &vec, Real theta);
    void rotate(Matrix3 const &R);
    void rotate(IncrementalRotation const &);

    void setRotation(Vec3 const &vec, Real theta);
    void setRotation(Matrix3 const &R);
    void setAzEl(Real az, Real el);
  };
}

#endif // _INCREMENTAL_ROTATION_H
