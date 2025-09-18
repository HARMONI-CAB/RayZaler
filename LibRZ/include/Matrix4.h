//
//  Copyright (c) 2025 Gonzalo José Carracedo Carballal
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

#ifndef _MATRIX4_H
#define _MATRIX4_H

#include "Vector4.h"
#include <iostream>

namespace RZ {
  struct Matrix4;

  static inline RZ::Matrix4 operator *(RZ::Real k, RZ::Matrix4 M);
  
  struct Matrix4 {
    union {
      Vec4 rows[4];
      struct {
        Vec4 vx, vy, vz, vt;
      } row;

      Real coef[4][4];
    };

    Matrix4() : Matrix4(Vec4::eX(), Vec4::eY(), Vec4::eZ(), Vec4::eT()) {}

    Matrix4(Vec4 const &row1, Vec4 const &row2, Vec4 const &row3, Vec4 const &row4)
    {
      row.vx = row1;
      row.vy = row2;
      row.vz = row3;
      row.vt = row4;
    }

    Matrix4(const RZ::Real coef[4][4])
    {
      memcpy(this->coef, coef, 4 * 4 * sizeof (RZ::Real));
    }

    inline Vec4 const &
    vx() const
    {
      return row.vx;
    }

    inline Vec4 const &
    vy() const
    {
      return row.vy;
    }

    inline Vec4 const &
    vz() const
    {
      return row.vz;
    }

    inline Vec4 const &
    vt() const
    {
      return row.vt;
    }

    // In-place apply (left)
    inline void
    applyLeft(Matrix4 const &m)
    {
      *this = m * *this;
    }

    // In-place apply (right)
    inline void
    applyRight(Matrix4 const &m)
    {
      *this = *this * m;
    }

    // Matrix-vector product
    inline Vec4
    operator *(Vec4 const &v) const
    {
      return Vec4(row.vx * v, row.vy * v, row.vz * v, row.vt * v);
    }

    // Matrix-matrix product
    inline Matrix4
    operator *(Matrix4 const &m) const
    {
      Vec4 cols[] = {
        Vec4(m.row.vx.x, m.row.vy.x, m.row.vz.x, m.row.vt.x),
        Vec4(m.row.vx.y, m.row.vy.y, m.row.vz.y, m.row.vt.y),
        Vec4(m.row.vx.z, m.row.vy.z, m.row.vz.z, m.row.vt.z),
        Vec4(m.row.vx.t, m.row.vy.t, m.row.vz.t, m.row.vt.t)
      };

      return Matrix4(
        Vec4(row.vx * cols[0], row.vx * cols[1], row.vx * cols[2], row.vx * cols[3]),
        Vec4(row.vy * cols[0], row.vy * cols[1], row.vy * cols[2], row.vy * cols[3]),
        Vec4(row.vz * cols[0], row.vz * cols[1], row.vz * cols[2], row.vz * cols[3]),
        Vec4(row.vt * cols[0], row.vt * cols[1], row.vt * cols[2], row.vt * cols[3])
      );
    }

    // Matrix inversion
    // Explicit algoritm obtained and tested from https://stackoverflow.com/questions/2624422/efficient-4x4-matrix-inverse-affine-transform
    bool
    invert(Matrix4 &out)
    {
      Matrix4 inverted = Matrix4::eye();
      
      auto s0 = coef[0][0] * coef[1][1] - coef[1][0] * coef[0][1];
      auto s1 = coef[0][0] * coef[1][2] - coef[1][0] * coef[0][2];
      auto s2 = coef[0][0] * coef[1][3] - coef[1][0] * coef[0][3];
      auto s3 = coef[0][1] * coef[1][2] - coef[1][1] * coef[0][2];
      auto s4 = coef[0][1] * coef[1][3] - coef[1][1] * coef[0][3];
      auto s5 = coef[0][2] * coef[1][3] - coef[1][2] * coef[0][3];

      auto c5 = coef[2][2] * coef[3][3] - coef[3][2] * coef[2][3];
      auto c4 = coef[2][1] * coef[3][3] - coef[3][1] * coef[2][3];
      auto c3 = coef[2][1] * coef[3][2] - coef[3][1] * coef[2][2];
      auto c2 = coef[2][0] * coef[3][3] - coef[3][0] * coef[2][3];
      auto c1 = coef[2][0] * coef[3][2] - coef[3][0] * coef[2][2];
      auto c0 = coef[2][0] * coef[3][1] - coef[3][0] * coef[2][1];

      // Should check for 0 determinant
      auto det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;

      if (isZero(det))
        return false;

      auto invdet = 1.0 / det;
      
      out.coef[0][0] = ( coef[1][1] * c5 - coef[1][2] * c4 + coef[1][3] * c3) * invdet;
      out.coef[0][1] = (-coef[0][1] * c5 + coef[0][2] * c4 - coef[0][3] * c3) * invdet;
      out.coef[0][2] = ( coef[3][1] * s5 - coef[3][2] * s4 + coef[3][3] * s3) * invdet;
      out.coef[0][3] = (-coef[2][1] * s5 + coef[2][2] * s4 - coef[2][3] * s3) * invdet;

      out.coef[1][0] = (-coef[1][0] * c5 + coef[1][2] * c2 - coef[1][3] * c1) * invdet;
      out.coef[1][1] = ( coef[0][0] * c5 - coef[0][2] * c2 + coef[0][3] * c1) * invdet;
      out.coef[1][2] = (-coef[3][0] * s5 + coef[3][2] * s2 - coef[3][3] * s1) * invdet;
      out.coef[1][3] = ( coef[2][0] * s5 - coef[2][2] * s2 + coef[2][3] * s1) * invdet;

      out.coef[2][0] = ( coef[1][0] * c4 - coef[1][1] * c2 + coef[1][3] * c0) * invdet;
      out.coef[2][1] = (-coef[0][0] * c4 + coef[0][1] * c2 - coef[0][3] * c0) * invdet;
      out.coef[2][2] = ( coef[3][0] * s4 - coef[3][1] * s2 + coef[3][3] * s0) * invdet;
      out.coef[2][3] = (-coef[2][0] * s4 + coef[2][1] * s2 - coef[2][3] * s0) * invdet;

      out.coef[3][0] = (-coef[1][0] * c3 + coef[1][1] * c1 - coef[1][2] * c0) * invdet;
      out.coef[3][1] = ( coef[0][0] * c3 - coef[0][1] * c1 + coef[0][2] * c0) * invdet;
      out.coef[3][2] = (-coef[3][0] * s3 + coef[3][1] * s1 - coef[3][2] * s0) * invdet;
      out.coef[3][3] = ( coef[2][0] * s3 - coef[2][1] * s1 + coef[2][2] * s0) * invdet;

      return true;
    }

    // Matrix-scalar product
    inline Matrix4
    operator *(Real k) const
    {
      return Matrix4(k * row.vx, k * row.vy, k * row.vz, k * row.vt);
    }

    // Matrix-scalar division
    inline Matrix4
    operator /(Real k) const
    {
      return *this * (1. / k);
    }

    // Matrix addition
    inline Matrix4
    operator +(Matrix4 const &m)
    {
      return Matrix4(
        row.vx + m.row.vx,
        row.vy + m.row.vy,
        row.vz + m.row.vz,
        row.vt + m.row.vt);
    }

    // Matrix subtraction
    inline Matrix4
    operator -(Matrix4 const &m)
    {
      return Matrix4(
        row.vx - m.row.vx,
        row.vy - m.row.vy,
        row.vz - m.row.vz,
        row.vt - m.row.vt);
    }

    // Determinant. Hardcoded from https://stackoverflow.com/questions/2937702/i-want-to-find-determinant-of-4x4-matrix-in-c-sharp
    inline Real
    det() const
    {
      return
         coef[0][3] * coef[1][2] * coef[2][1] * coef[3][0] - coef[0][2] * coef[1][3] * coef[2][1] * coef[3][0] -
         coef[0][3] * coef[1][1] * coef[2][2] * coef[3][0] + coef[0][1] * coef[1][3] * coef[2][2] * coef[3][0] +
         coef[0][2] * coef[1][1] * coef[2][3] * coef[3][0] - coef[0][1] * coef[1][2] * coef[2][3] * coef[3][0] -
         coef[0][3] * coef[1][2] * coef[2][0] * coef[3][1] + coef[0][2] * coef[1][3] * coef[2][0] * coef[3][1] +
         coef[0][3] * coef[1][0] * coef[2][2] * coef[3][1] - coef[0][0] * coef[1][3] * coef[2][2] * coef[3][1] -
         coef[0][2] * coef[1][0] * coef[2][3] * coef[3][1] + coef[0][0] * coef[1][2] * coef[2][3] * coef[3][1] +
         coef[0][3] * coef[1][1] * coef[2][0] * coef[3][2] - coef[0][1] * coef[1][3] * coef[2][0] * coef[3][2] -
         coef[0][3] * coef[1][0] * coef[2][1] * coef[3][2] + coef[0][0] * coef[1][3] * coef[2][1] * coef[3][2] +
         coef[0][1] * coef[1][0] * coef[2][3] * coef[3][2] - coef[0][0] * coef[1][1] * coef[2][3] * coef[3][2] -
         coef[0][2] * coef[1][1] * coef[2][0] * coef[3][3] + coef[0][1] * coef[1][2] * coef[2][0] * coef[3][3] +
         coef[0][2] * coef[1][0] * coef[2][1] * coef[3][3] - coef[0][0] * coef[1][2] * coef[2][1] * coef[3][3] -
         coef[0][1] * coef[1][0] * coef[2][2] * coef[3][3] + coef[0][0] * coef[1][1] * coef[2][2] * coef[3][3];
    }

    // Trace
    inline Real
    tr() const
    {
      return coef[0][0] + coef[1][1] + coef[2][2] + coef[3][3];
    }

    // Transpose
    inline Matrix4
    t() const
    {
      return Matrix4(
        Vec4(row.vx.x, row.vy.x, row.vz.x, row.vt.x),
        Vec4(row.vx.y, row.vy.y, row.vz.y, row.vt.y),
        Vec4(row.vx.z, row.vy.z, row.vz.z, row.vt.z),
        Vec4(row.vx.t, row.vy.t, row.vz.t, row.vt.t)
      );
    }

    // Common matrices
    static inline Matrix4
    zero()
    {
      return Matrix4(Vec4::zero(), Vec4::zero(), Vec4::zero(), Vec4::zero());
    }

    static inline Matrix4
    eye()
    {
      return Matrix4(Vec4::eX(), Vec4::eY(), Vec4::eZ(), Vec4::eT());
    }

    inline std::string
    toString() const
    {
      return std::string("[\n")
      + "  " + this->row.vx.toString() + "\n"
      + "  " + this->row.vy.toString() + "\n"
      + "  " + this->row.vz.toString() + "\n"
      + "  " + this->row.vt.toString() + "\n"
      + "]";
    }

    inline bool
    operator==(Matrix4 const &other) const
    {
      return 
           (this->row.vx == other.row.vx)
        && (this->row.vy == other.row.vy)
        && (this->row.vz == other.row.vz)
        && (this->row.vt == other.row.vt);
    }
  };

  static inline RZ::Matrix4
  operator *(RZ::Real k, RZ::Matrix4 M)
  {
    return M * k;
  }
}

#endif // _MATRIX4
