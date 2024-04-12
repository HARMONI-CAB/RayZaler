#ifndef _APERTURES_ARRAY_H
#define _APERTURES_ARRAY_H

#include <GenericAperture.h>

namespace RZ {
  class ApertureArray : public GenericAperture {
    GenericAperture *m_subAperture = nullptr;
    Real m_width             = 100e-3;
    Real m_height            = 100e-3;
    unsigned int m_rows      = 10;
    unsigned int m_cols      = 10;
    Real m_subApertureWidth  = 10e-3;
    Real m_subApertureHeight = 10e-3;
    void recalculateDimensions();

  public:
    inline GenericAperture *subAperture() const
    {
      return m_subAperture;
    }

    template <class T>
    inline T *subAperture()
    {
      return static_cast<T *>(subAperture());
    }
    
    template <class T>
    inline T const *subAperture() const
    {
      return static_cast<const T *>(subAperture());
    }

    inline Real
    subApertureWidth() const
    {
      return m_subApertureWidth;
    }

    inline Real
    subApertureHeight() const
    {
      return m_subApertureHeight;
    }

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
    
    ApertureArray(GenericAperture *);
    virtual ~ApertureArray();

    void setWidth(Real);
    void setHeight(Real);
    void setCols(unsigned);
    void setRows(unsigned);

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

#endif // _APERTURES_ARRAY_H
