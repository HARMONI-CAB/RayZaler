#ifndef _MATRIX_H
#define _MATRIX_H

#include "Vector.h"
#include <iostream>

namespace RZ {
  // 3-D matrices are actually a bunch of 3 Vec3

  static inline Real
  deg2rad(Real deg)
  {
    Real rad = (deg / 180. + 1) * M_PI;
    return rad - 2 * M_PI * floor(rad / (2 * M_PI)) - M_PI;
  }

  static inline Real
  rad2deg(Real rad)
  {
    Real deg = (rad / M_PI + 1) * 180.;
    return deg - 360. * floor(rad / 360.) - 180.;
  }

  struct Matrix3;

  static inline RZ::Matrix3 operator *(RZ::Real k, RZ::Matrix3 M);
  
  struct Matrix3 {
    union {
      Vec3 rows[3];
      struct {
        Vec3 vx, vy, vz;
      } row;

      Real coef[3][3];
    };

    Matrix3() : Matrix3(Vec3::eX(), Vec3::eY(), Vec3::eZ()) {}

    Matrix3(Vec3 const &row1, Vec3 const &row2, Vec3 const &row3)
    {
      row.vx = row1;
      row.vy = row2;
      row.vz = row3;
    }

    Matrix3(const RZ::Real coef[3][3])
    {
      memcpy(this->coef, coef, 3 * 3 * sizeof (RZ::Real));
    }

    inline Vec3 const &
    vx() const
    {
      return row.vx;
    }

    inline Vec3 const &
    vy() const
    {
      return row.vy;
    }

    inline Vec3 const &
    vz() const
    {
      return row.vz;
    }

    // In-place apply (left)
    inline void
    applyLeft(Matrix3 const &m)
    {
      *this = m * *this;
    }

    // In-place apply (right)
    inline void
    applyRight(Matrix3 const &m)
    {
      *this = *this * m;
    }

    // Matrix-vector product
    inline Vec3
    operator *(Vec3 const &v) const
    {
      return Vec3(row.vx * v, row.vy * v, row.vz * v);
    }

    // Matrix-matrix product
    inline Matrix3
    operator *(Matrix3 const &m) const
    {
      Vec3 cols[] = {
        Vec3(m.row.vx.x, m.row.vy.x, m.row.vz.x),
        Vec3(m.row.vx.y, m.row.vy.y, m.row.vz.y),
        Vec3(m.row.vx.z, m.row.vy.z, m.row.vz.z)
      };

      return Matrix3(
        Vec3(row.vx * cols[0], row.vx * cols[1], row.vx * cols[2]),
        Vec3(row.vy * cols[0], row.vy * cols[1], row.vy * cols[2]),
        Vec3(row.vz * cols[0], row.vz * cols[1], row.vz * cols[2])
      );
    }

    // Matrix-scalar product
    inline Matrix3
    operator *(Real k) const
    {
      return Matrix3(k * row.vx, k * row.vy, k * row.vz);
    }

    // Matrix-scalar division
    inline Matrix3
    operator /(Real k) const
    {
      return *this * (1. / k);
    }

    // Matrix addition
    inline Matrix3
    operator +(Matrix3 const &m)
    {
      return Matrix3(row.vx + m.row.vx, row.vy + m.row.vy, row.vz + m.row.vz);
    }

    // Matrix subtraction
    inline Matrix3
    operator -(Matrix3 const &m)
    {
      return Matrix3(row.vx - m.row.vx, row.vy - m.row.vy, row.vz - m.row.vz);
    }

    // Determinant
    inline Real
    det() const
    {
      return
      row.vx.x * row.vy.y * row.vz.z + row.vx.y * row.vy.z * row.vz.x + row.vx.z * row.vy.x * row.vz.y
      - (row.vx.x * row.vy.z * row.vz.y + row.vx.y * row.vy.x * row.vz.z + row.vx.z * row.vy.y * row.vz.x);
    }

    // Trace
    inline Real
    tr() const
    {
      return coef[0][0] + coef[1][1] + coef[2][2];
    }

    // Transpose
    inline Matrix3
    t() const
    {
      return Matrix3(
        Vec3(row.vx.x, row.vy.x, row.vz.x),
        Vec3(row.vx.y, row.vy.y, row.vz.y),
        Vec3(row.vx.z, row.vy.z, row.vz.z)
      );
    }

    // Common matrices
    static inline Matrix3
    zero()
    {
      return Matrix3(Vec3::zero(), Vec3::zero(), Vec3::zero());
    }

    static inline Matrix3
    eye()
    {
      return Matrix3(Vec3::eX(), Vec3::eY(), Vec3::eZ());
    }

    static inline Matrix3
    crossMatrix(Vec3 const &k)
    {
      return Matrix3(
        Vec3(0,  -k.z, +k.y),
        Vec3(+k.z,  0, -k.x),
        Vec3(-k.y,  +k.x,   0)
      );
    }

    static inline Matrix3
    rot(Vec3 const &k, Real theta)
    {
      Matrix3 K = crossMatrix(k);
      return eye() + K * sin(theta) + (1 - cos(theta)) * K * K;
    }

    //
    // The AzEl orientation matrix works as follows:
    // X points towards the North
    // Y points towards the West
    // Z points towards the Zenith
    //

    static inline Matrix3
    azel(Real az, Real el)
    {
      return Matrix3::rot(Vec3::eY(), M_PI / 2 - el) * Matrix3::rot(Vec3::eZ(), -az);
    }

    inline std::string
    toString() const
    {
      return std::string("[\n")
      + "  " + this->row.vx.toString() + "\n"
      + "  " + this->row.vy.toString() + "\n"
      + "  " + this->row.vz.toString() + "\n"
      + "]";
    }

    inline bool
    operator==(Matrix3 const &other) const
    {
      return 
           (this->row.vx == other.row.vx)
        && (this->row.vy == other.row.vy)
        && (this->row.vz == other.row.vz);
    }
  };

  static inline RZ::Matrix3
  operator *(RZ::Real k, RZ::Matrix3 M)
  {
    return M * k;
  }
}

#endif // _MATRIX
