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
    void setRotation(Vec3 const &vec, Real theta);
    void setAzEl(Real az, Real el);
  };
}

#endif // _INCREMENTAL_ROTATION_H
