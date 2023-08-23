#ifndef _BLOCKELEMENT_H
#define _BLOCKELEMENT_H

#include <Element.h>
#include <GL/glu.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;
  class RotatedFrame;

  class BlockElement : public Element {
      TranslatedFrame *m_sides[6];
      RotatedFrame *m_rotatedSides[6];

      Real m_cachedLength, m_cachedWidth, m_cachedHeight;
      void initSides();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      BlockElement(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~BlockElement();
      virtual void renderOpenGL() override;
  };

  class BlockElementFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _BlockELEMENT_H
