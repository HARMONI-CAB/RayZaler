#ifndef _VECTOR_H
#define _VECTOR_H

#include <cmath>
#include <string>
#include <ostream>
#include <cstring>

#define RZ_DEFAULT_COMPARE_RELATIVE_ERROR 1e-9
#define RZ_URANDSIGN (2 * (static_cast<Real>(rand()) / RAND_MAX - .5))

namespace RZ {
  typedef double Real;

  struct Vec3;
  typedef Vec3 Point3;
  static inline RZ::Vec3 operator *(RZ::Real k, RZ::Vec3 v);

  static inline bool
  releq(Real a, Real b, Real precision = 1e-9)
  {
    return fabs(a - b) / fabs(b) < precision;
  }

  static inline bool
  isZero(Real a, Real precision = 1e-9)
  {
    return fabs(a) < precision;
  }

  struct Vec3 {
    union {
      struct {
        Real x, y, z;
      };

      Real coords[3];
    };

    inline Vec3() : Vec3(0, 0, 0) { }
    inline Vec3(Real x, Real y, Real z) : x(x), y(y), z(z) { }
    inline Vec3(const Real *coords) : x(coords[0]), y(coords[1]), z(coords[2]) {}

    // Zero
    static inline Vec3
    zero()
    {
      return Vec3(0, 0, 0);
    }

    // Basis vectors
    static inline Vec3
    eX()
    {
      return Vec3(1, 0, 0);
    }

    // Basis vectors
    static inline Vec3
    eY()
    {
      return Vec3(0, 1, 0);
    }
    
    // Basis vectors
    static inline Vec3
    eZ()
    {
      return Vec3(0, 0, 1);
    }
    
    // Dot product
    inline Real
    operator * (Vec3 const &v) const
    {
      return x * v.x + y * v.y + z * v.z;
    }

    // Vector summation
    inline Vec3
    operator + (Vec3 const &v) const
    {
      return Vec3(x + v.x, y + v.y, z + v.z);
    }

    // Vector subtraction
    inline Vec3
    operator - (Vec3 const &v) const
    {
      return Vec3(x - v.x, y - v.y, z - v.z);
    }

    // Product by constant
    inline Vec3
    operator * (Real k) const
    {
      return Vec3(k * x, k * y, k * z);
    }

    // Inline add
    inline Vec3 &
    operator += (const Vec3 &vec) {
      x += vec.x;
      y += vec.y;
      z += vec.z;

      return *this;
    }

    // Inline subtract
    inline Vec3 &
    operator -= (const Vec3 &vec) {
      x -= vec.x;
      y -= vec.y;
      z -= vec.z;

      return *this;
    }
    
    // Cross product
    inline Vec3
    cross(Vec3 const &v) const
    {
      return Vec3(
        y * v.z - z * v.y,
        z * v.x - x * v.z,
        x * v.y - y * v.x);
    }

    // 2-norm
    inline Real
    norm() const
    {
      return sqrt(x * x + y * y + z * z);
    }

    // Normalized version
    inline Vec3
    normalized() const
    {
      Real k = 1 / norm();

      return *this * k;
    }

    inline bool
    compare(Vec3 const &other, Real dist) const
    {
      return (other - *this).norm() < dist;
    }

    inline bool
    compare(Vec3 const &other) const
    {
      return this->compare(other, std::numeric_limits<Real>::epsilon());
    }

    inline bool
    operator== (Vec3 const &other) const
    {
      Real norm = this->norm();
      if (norm < std::numeric_limits<Real>::epsilon())
        return other.norm() < std::numeric_limits<Real>::epsilon();

      return (other - *this).norm() / norm < RZ_DEFAULT_COMPARE_RELATIVE_ERROR;
    }

    inline bool
    operator!= (Vec3 const &other) const
    {
      return !(*this == other);
    }

    inline Vec3
    operator-() const
    {
      return Vec3(-this->x, -this->y, -this->z);
    }

    inline void
    copyToArray(Real *dest)
    {
      memcpy(dest, this->coords, 3 * sizeof(Real));
    }

    std::string
    toString() const
    {
      return 
        "(" + std::to_string(this->x) 
      + "," + std::to_string(this->y) 
      + "," + std::to_string(this->z)
      + ")";
    }
  };

  static inline RZ::Vec3
  operator *(RZ::Real k, RZ::Vec3 v)
  {
    return v * k;
  }
}

inline std::ostream&
operator<<(std::ostream& os, const RZ::Vec3& vec)
{
    os << vec.toString();
    return os;
}

#endif // _VECTOR_H
