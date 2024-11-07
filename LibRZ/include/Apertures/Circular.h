#ifndef _APERTURES_CIRCULAR_H
#define _APERTURES_CIRCULAR_H

#include <GLHelpers.h>
#include <GenericAperture.h>

namespace RZ {
  class CircularAperture : public GenericAperture {
    Real m_radius;
    Real m_radius2;
    Real m_a  = 1;
    Real m_b  = 1;
    Real m_a2 = 1;
    Real m_b2 = 1;
    bool m_obstruction = false;

    std::vector<GLfloat> m_vertices;
    std::vector<GLfloat> m_grid;
    std::vector<std::vector<Real>> m_edges;

    void recalculate();
    void renderOpenGLAperture();
    void renderOpenGLObstruction();
    
  public:
    CircularAperture(Real radius);

    virtual std::vector<std::vector<Real>> const &edges() const override;
    void setRadius(Real);
    void setEccentricity(Real);

    // 
    // Calculate radius and eccentricity from width and height. Given that:
    //
    // x^2 / A^2 + y^2 / B^2 = 1
    //
    // With A = aR and B = bR, then:
    //
    // width  = 2A = 2aR
    // height = 2B = 2bR
    //
    // Since we want to make sure that pi R^2 = piAB, then:
    // 
    // pi width * height / 4 = pi R^2 => R = sqrt(width * height) / 2
    // 
    static inline void
    radiusEccentricity(Real &R, Real &e, Real width, Real height)
    {
      Real a2, b2;

      R   = .5 * sqrt(width * height);
      a2  = .5 * width  / R;
      a2 *= a2;
      b2  = .5 * height / R;
      b2 *= b2;

      if (b2 < a2)
        e = +sqrt(1 - b2 * b2);
      else
        e = -sqrt(1 - a2 * a2);
    }

    inline Real
    a() const
    {
      return sqrt(m_a2);
    }

    inline Real
    b() const
    {
      return sqrt(m_b2);
    }

    inline Real
    a2() const
    {
      return m_a2;
    }

    inline Real
    b2() const
    {
      return m_b2;
    }

    void setObstruction(bool);
    
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

#endif // _APERTURES_CIRCULAR_H
