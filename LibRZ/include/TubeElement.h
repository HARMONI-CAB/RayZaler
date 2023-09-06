#ifndef _TUBEELEMENT_H
#define _TUBEELEMENT_H

#include <Element.h>
#include <GL/glu.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;
  class RotatedFrame;

  class TubeElement : public Element {
      TranslatedFrame *m_sides[3];        // Top, middle, bottom
      RotatedFrame    *m_rotatedSides[3]; // Top, middle, bottom

      Real m_cachedLength = 5e-2;
      Real m_cachedOuterDiameter = 3e-3;
      Real m_cachedInnerDiameter = 1.5e-3;

      GLTube m_tube;

      void initSides();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      TubeElement(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~TubeElement();
      virtual void renderOpenGL() override;
  };

  class TubeElementFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _TUBEELEMENT_H
