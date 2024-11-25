#ifndef _SURFACES_RECTANGULAR_H
#define _SURFACES_RECTANGULAR_H

#include <SurfaceShape.h>

namespace RZ {
  class RectangularFlatSurface : public SurfaceShape {
    Real m_width             = 100e-3;
    Real m_height            = 100e-3;
    void updateEdges();
    std::vector<std::vector<Real>> m_edges;
    
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
    
    RectangularFlatSurface();
    virtual ~RectangularFlatSurface();

    void setWidth(Real);
    void setHeight(Real);
    virtual std::vector<std::vector<Real>> const &edges() const override;

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

#endif // _SURFACES_RECTANGULAR_H
