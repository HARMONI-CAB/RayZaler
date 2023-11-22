#ifndef _RECTANGULAR_STOP_H
#define _RECTANGULAR_STOP_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class RectangularStop : public OpticalElement {
      RectangularStopProcessor *m_processor;
      GLRectangle m_vRect, m_hRect;
      TranslatedFrame *m_stopSurface = nullptr;
      
      Real m_width  = 3e-2;
      Real m_height = 3e-2;

      Real m_borderWidth  = 4e-2;
      Real m_borderHeight = 4e-2;

      Real m_hShift, m_vShift;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      RectangularStop(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~RectangularStop();

      virtual void nativeMaterialOpenGL(std::string const &role) override;
      virtual void renderOpenGL() override;
  };

  class RectangularStopFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _RECTANGULAR_STOP_H
