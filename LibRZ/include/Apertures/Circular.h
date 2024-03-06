#ifndef _APERTURES_CIRCULAR_H
#define _APERTURES_CIRCULAR_H

#include <GenericAperture.h>

namespace RZ {
  class CircularAperture : public GenericAperture {
    Real m_radius2;

  public:
    CircularAperture(Real radius);
    void setRadius(Real);

    virtual bool intercept(
      Vec3 &coord,
      Vec3 &n,
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

#endif // _APERTURES_CIRCULAR_H
