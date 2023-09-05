#ifndef _TRIPOD_H
#define _TRIPOD_H

#include <Element.h>
#include <GL/glu.h>
#include <GLHelpers.h>
#include <TripodFrame.h>

namespace RZ {
  class TranslatedFrame;
  class RotatedFrame;

  class Tripod : public Element {
      GLCappedCylinder m_glLegs[3];
      TripodFrame     *m_surface;

      Real m_legDiameter = 6e-3;
      Real m_leg1        = 2e-2;
      Real m_leg2        = 2e-2;
      Real m_leg3        = 2e-2;
      Real m_ta_angle    = 70.; // Deg
      Real m_ta_radius   = 42e-3;

      // Calculated
      Vec3 m_p[3];

      void recalcLegs();
      void initTripod();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      Tripod(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~Tripod();
      virtual void renderOpenGL() override;
  };

  class TripodFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _TRIPOD_H
