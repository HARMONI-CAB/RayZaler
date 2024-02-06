#ifndef _APERTURES_PARABOLIC_H
#define _APERTURES_PARABOLIC_H

#include <GenericAperture.h>

namespace RZ {
  class ParabolicAperture : public GenericAperture {
    Real m_radius2;
    Real m_flength = 1;

  public:
    ParabolicAperture(Real radius, Real fLength);

    void setRadius(Real);
    void setFocalLength(Real);

    virtual bool intercept(
      Vec3 &hit,
      Vec3 &normal,
      Vec3 const &origin) const override;
    
    virtual void generatePoints(
        const ReferenceFrame *,
        Real *pointArr,
        unsigned int N) override;
  };
}

#endif // _APERTURES_PARABOLIC_H
