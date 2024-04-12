#ifndef _APERTURES_RECTANGULAR_H
#define _APERTURES_RECTANGULAR_H

#include <GenericAperture.h>

namespace RZ {
  class RectangularAperture : public GenericAperture {
    Real m_width             = 100e-3;
    Real m_height            = 100e-3;
    
  public:
    inline Real
    width() const
    {
      return m_width;
    }

    inline Real
    height() const
    {
      return m_height;
    }
    
    RectangularAperture();
    virtual ~RectangularAperture();

    void setWidth(Real);
    void setHeight(Real);

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

    virtual void renderOpenGL() override;
  };
}

#endif // _APERTURES_RECTANGULAR_H
