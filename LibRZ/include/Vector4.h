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

#ifndef _VECTOR4_H
#define _VECTOR4_H

#include <cmath>
#include <string>
#include <ostream>
#include <cstring>
#include <complex>
#include <Vector.h>

namespace RZ {
  struct Vec4;
  typedef Vec4 Point4;

  struct Vec4 {
    union {
      struct {
        Real x, y, z, t;
      };

      struct {
        Complex x, y;
      } c;

      struct {
        Real xr, xi, yr, yi;
      };

      Real coords[4];
    };

    inline Vec4() : Vec4(0, 0, 0, 0) { }
    inline Vec4(Real x, Real y, Real z, Real t) : x(x), y(y), z(z), t(t) { }
    inline Vec4(Complex x, Complex y) { c.x = x; c.y = y; }
    inline Vec4(const Real coords[4]) : 
      x(coords == nullptr ? 0 : coords[0]),
      y(coords == nullptr ? 0 : coords[1]),
      z(coords == nullptr ? 0 : coords[2]),
      t(coords == nullptr ? 0 : coords[3]) {}

    inline Vec4(const Complex coords[2])
    {
      c.x = coords == nullptr ? 0 : coords[0];
      c.y = coords == nullptr ? 0 : coords[1];
    }

    // Zero
    static inline Vec4
    zero()
    {
      return Vec4(0, 0, 0, 0);
    }

    // Basis vectors
    static inline Vec4
    eX()
    {
      return Vec4(1, 0, 0, 0);
    }

    static inline Vec4
    eCX()
    {
      return Vec4(1, 0, 0, 0);
    }

    // Basis vectors
    static inline Vec4
    eY()
    {
      return Vec4(0, 1, 0, 0);
    }
    
    // Basis vectors
    static inline Vec4
    eZ()
    {
      return Vec4(0, 0, 1, 0);
    }
    
    static inline Vec4
    eCY()
    {
      return Vec4(0, 0, 1, 0);
    }

    // Basis vectors
    static inline Vec4
    eT()
    {
      return Vec4(0, 0, 0, 1);
    }

    // Dot product
    inline Real
    operator * (Vec4 const &v) const
    {
      return x * v.x + y * v.y + z * v.z + t * v.t;
    }

    // Vector summation
    inline Vec4
    operator + (Vec4 const &v) const
    {
      return Vec4(c.x + v.c.x, c.y + v.c.y);
    }

    // Vector subtraction
    inline Vec4
    operator - (Vec4 const &v) const
    {
      return Vec4(c.x - v.c.x, c.y - v.c.y);
    }

    // Product by scalar
    inline Vec4
    operator * (Real k) const
    {
      return Vec4(k * c.x, k * c.y);
    }

    // Product by complex
    inline Vec4
    operator * (Complex const &z) const
    {
      return Vec4(z * c.x, z * c.y);
    }

    // Division by scalar
    inline Vec4
    operator / (Real k) const
    {
      return *this * (1. / k);
    }

    // Division by complex
    inline Vec4
    operator / (Complex const &z) const
    {
      return *this * (1. / z);
    }

    // Inline add
    inline Vec4 &
    operator += (const Vec4 &vec) {
      c.x += vec.c.x;
      c.y += vec.c.y;

      return *this;
    }

    // Inline subtract
    inline Vec4 &
    operator -= (const Vec4 &vec) {
      c.x -= vec.c.x;
      c.y -= vec.c.y;

      return *this;
    }
    
    // 2-norm
    inline Real
    norm() const
    {
      return sqrt(x * x + y * y + z * z + t * t);
    }

    // Complex dot product
    inline Complex
    sdot(Vec4 const &v) const
    {
      return std::conj(v.c.x) * c.x + std::conj(v.c.y) * c.y;
    }

    // Normalized version
    inline Vec4
    normalized() const
    {
      Real k = 1 / norm();

      return *this * k;
    }

    // Check if is null
    inline bool
    isNull(Real tol = 1e-9) const
    {
      return isZero(x, tol) && isZero(y, tol) && isZero(z, tol) && isZero(t, tol);
    }

    inline bool
    compare(Vec4 const &other, Real dist) const
    {
      return (other - *this).norm() < dist;
    }

    inline bool
    compare(Vec4 const &other) const
    {
      return this->compare(other, std::numeric_limits<Real>::epsilon());
    }

    inline bool
    operator== (Vec4 const &other) const
    {
      Real norm = this->norm();
      if (norm < std::numeric_limits<Real>::epsilon())
        return other.norm() < std::numeric_limits<Real>::epsilon();

      return (other - *this).norm() / norm < RZ_DEFAULT_COMPARE_RELATIVE_ERROR;
    }

    inline bool
    operator!= (Vec4 const &other) const
    {
      return !(*this == other);
    }

    inline Vec4
    operator-() const
    {
      return Vec4(-this->c.x, -this->c.y);
    }

    inline void
    copyToArray(Real *dest) const
    {
      memcpy(dest, this->coords, 4 * sizeof(Real));
    }

    inline void
    copyToArray(Complex *dest) const
    {
      memcpy(dest, this->coords, 4 * sizeof(Real));
    }

    inline void
    setFromArray(const Real *coords)
    {
      memcpy(this->coords, coords, 4 * sizeof(Real));
    }

    inline void
    setFromArray(const Complex *coords)
    {
      memcpy(this->coords, coords, 4 * sizeof(Real));
    }

    std::string
    toString() const
    {
      return 
        "(" + std::to_string(this->x) 
      + "," + std::to_string(this->y) 
      + "," + std::to_string(this->z)
      + "," + std::to_string(this->t)
      + ")";
    }

    inline Vec4 &
    operator=(RZ:: Real v)
    {
      coords[0] = coords[1] = coords[2] = coords[3] = v;
      return *this;
    }

    inline Vec4 &
    operator=(RZ:: Complex v)
    {
      c.x = c.y = v;
      return *this;
    }
  };

  static inline RZ::Vec4
  operator *(RZ::Real k, RZ::Vec4 v)
  {
    return v * k;
  }

  static inline RZ::Vec4
  operator *(RZ::Complex k, RZ::Vec4 v)
  {
    return v * k;
  }
}

inline std::ostream&
operator<<(std::ostream& os, const RZ::Vec4& vec)
{
    os << vec.toString();
    return os;
}

#endif // _VECTOR4_H
