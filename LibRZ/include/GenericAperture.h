#ifndef _GENERIC_APERTURE_H
#define _GENERIC_APERTURE_H

#include "Vector.h"
#include "ReferenceFrame.h"
#include <Random.h>

#define GENERIC_APERTURE_NUM_SEGMENTS     36

namespace RZ {
  class GenericAperture {
      ExprRandomState m_state;

    public:
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

      virtual void renderOpenGL();
  };
}

#endif // _GENERIC_APERTURE_H
