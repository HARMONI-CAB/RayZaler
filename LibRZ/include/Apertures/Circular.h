#ifndef _APERTURES_CIRCULAR_H
#define _APERTURES_CIRCULAR_H

#include <GLHelpers.h>
#include <GenericAperture.h>

namespace RZ {
  class CircularAperture : public GenericAperture {
    Real m_radius;
    Real m_radius2;
    std::vector<GLfloat> m_vertices;

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

    virtual void renderOpenGL() override;
  };
}

#endif // _APERTURES_CIRCULAR_H
