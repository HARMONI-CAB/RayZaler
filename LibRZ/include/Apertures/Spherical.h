#ifndef _APERTURES_SPHERICAL_H
#define _APERTURES_SPHERICAL_H

#include <GLHelpers.h>
#include <GenericAperture.h>

namespace RZ {
  class SphericalAperture : public GenericAperture {
    Real m_radius;
    Real m_radius2;
    Real m_rCurv  =  1;
    bool m_convex = true;
    Real m_center = .866;
    Real m_K = 1.; // Normalization constant for generatePoints
    Real m_rCurv2 = 1;
    Real m_KRcInv = 1.;
    Real m_x0 = 0;
    Real m_y0 = 0;
    Real m_ux = 1;
    Real m_uy = 0;

    std::vector<GLfloat> m_vertices;
    std::vector<GLfloat> m_axes;

    void recalcGL();
    void recalcDistribution();

  public:
    inline Real
    centerOffset() const
    {
      return m_convex ? +m_center : -m_center;
    }

    SphericalAperture(Real radius, Real rCurv);

    void setCenterOffset(Real, Real);
    void setConvex(bool);
    void setRadius(Real);
    void setCurvatureRadius(Real);

    virtual bool intercept(
      Vec3 &hit,
      Vec3 &normal,
      Real &tIgnore,
      Vec3 const &origin) const override;
    
    virtual Real area() const override;
    
    virtual void generatePoints(
        const ReferenceFrame *,
        Real *pointArr,
        Real *normalArr,
        unsigned int N) override;

    virtual void renderOpenGL() override;
  };
}

#endif // _APERTURES_SPHERICAL_H
