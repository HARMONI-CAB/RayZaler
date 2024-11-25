#ifndef _SURFACES_PARABOLIC_H
#define _SURFACES_PARABOLIC_H

#include <SurfaceShape.h>
#include <GLHelpers.h>

namespace RZ {
  class ParabolicSurface : public SurfaceShape {
    Real m_radius;
    Real m_radius2;
    Real m_flength = 1;
    Real m_4f2, m_8f3;
    Real m_6f_K;
    Real m_depth;
    Real m_x0 = 0;
    Real m_y0 = 0;
    
    Real m_ux = 1;
    Real m_uy = 0;
    
    std::vector<GLfloat> m_vertices;
    std::vector<GLfloat> m_axes;
    std::vector<std::vector<Real>> m_edges;
    
    void recalcGL();
    void recalcDistribution();

  public:
    ParabolicSurface(Real radius, Real fLength);
    virtual std::vector<std::vector<Real>> const &edges() const override;

    void setRadius(Real);
    void setFocalLength(Real);
    void setCenterOffset(Real, Real);

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

    virtual void renderOpenGL() override;
  };
}

#endif // _SURFACES_PARABOLIC_H
