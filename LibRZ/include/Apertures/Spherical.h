#ifndef _APERTURES_SPHERICAL_H
#define _APERTURES_SPHERICAL_H

#include <GenericAperture.h>

namespace RZ {
  class SphericalAperture : public GenericAperture {
    Real m_radius2;
    Real m_rCurv  =  1;
    bool m_convex = true;
    Real m_center = .866;

  public:
    inline Real
    centerOffset() const
    {
      return m_convex ? +m_center : -m_center;
    }

    SphericalAperture(Real radius, Real rCurv);

    void setConvex(bool);
    void setRadius(Real);
    void setCurvatureRadius(Real);

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

#endif // _APERTURES_SPHERICAL_H
