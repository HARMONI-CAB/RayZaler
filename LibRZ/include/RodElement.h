#ifndef _RODELEMENT_H
#define _RODELEMENT_H

#include <Element.h>
#include <GL/glu.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;
  class RotatedFrame;

  class RodElement : public Element {
      TranslatedFrame *m_sides[3];        // Top, middle, bottom
      RotatedFrame    *m_rotatedSides[3]; // Top, middle, bottom

      Real m_cachedLength = 5e-2;
      Real m_cachedDiameter = 3e-3;

      GLCappedCylinder m_cylinder;

      void initSides();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      RodElement(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~RodElement();
      virtual void renderOpenGL() override;
  };

  class RodElementFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _RODELEMENT_H
