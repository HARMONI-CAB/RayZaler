#ifndef _OBSTRUCTION_H
#define _OBSTRUCTION_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class Obstruction : public OpticalElement {
      ObstructionProcessor *m_processor;
      GLDisc                m_disc;
      TranslatedFrame      *m_stopSurface = nullptr;
      Real m_radius = 1e-2;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      Obstruction(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~Obstruction();

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  class ObstructionFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _OBSTRUCTION_H
