#ifndef _BENCHELEMENT_H
#define _BENCHELEMENT_H

#include <Element.h>
#include <GL/glu.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;
  class BenchElement : public Element {
      TranslatedFrame *m_surfaceFrame = nullptr;
      Real m_cachedHeight             = 0;
      GLCappedCylinder m_cylinder;
      
    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      BenchElement(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~BenchElement();
      virtual void renderOpenGL() override;
  };

  class BenchElementFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _BENCHELEMENT_H
