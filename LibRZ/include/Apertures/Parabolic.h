#ifndef _APERTURES_PARABOLIC_H
#define _APERTURES_PARABOLIC_H

#include <GenericAperture.h>

namespace RZ {
  class ParabolicAperture : public GenericAperture {
    Real m_radius2;
    Real m_flength = 1;
    Real m_4f2, m_8f3;
    Real m_6f_K;
    Real m_depth;

    void recalcDistribution();

  public:
    ParabolicAperture(Real radius, Real fLength);

    void setRadius(Real);
    void setFocalLength(Real);

    virtual bool intercept(
      Vec3 &hit,
      Vec3 &normal,
      Real &tIgnore,
      Vec3 const &origin) const override;
    
    virtual Real area() const override;
    
    virtual void generatePoints(
        const ReferenceFrame *,
        Real *pointArr,
        Real *normals,
        unsigned int N) override;
  };
}

#endif // _APERTURES_PARABOLIC_H
