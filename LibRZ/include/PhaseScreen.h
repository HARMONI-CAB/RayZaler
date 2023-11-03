#ifndef _PHASE_SCREEN_H
#define _PHASE_SCREEN_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>


namespace RZ {
  class TranslatedFrame;

  class PhaseScreen : public OpticalElement {
      PhaseScreenProcessor *m_processor;
      GLPinHole m_pinHole;
      TranslatedFrame *m_stopSurface = nullptr;
      Real m_muIn    = 1;
      Real m_muOut   = 1.5;
      Real m_radius  = 2.5e-2;
      Real m_width   = 7.5e-2;
      Real m_height  = 7.5e-2;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      PhaseScreen(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~PhaseScreen();

      virtual void nativeMaterialOpenGL(std::string const &role) override;
      virtual void renderOpenGL() override;
  };

  class PhaseScreenFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}


#endif // _PHASE_SCREEN_H
