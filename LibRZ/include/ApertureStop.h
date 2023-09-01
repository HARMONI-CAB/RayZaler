#ifndef _APERTURE_STOP_H
#define _APERTURE_STOP_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class ApertureStop : public OpticalElement {
      ApertureStopProcessor *m_processor;
      GLPinHole m_pinHole;
      TranslatedFrame *m_stopSurface = nullptr;
      Real m_radius = 1e-2;
      Real m_width  = 3e-2;
      Real m_height = 3e-2;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      ApertureStop(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~ApertureStop();

      virtual void nativeMaterialOpenGL(std::string const &role) override;
      virtual void renderOpenGL() override;
  };

  class ApertureStopFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _APERTURE_STOP_H
