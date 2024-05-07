#ifndef _CIRCULAR_WINDOW_H
#define _CIRCULAR_WINDOW_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;
  class CircularWindowProcessor;

  class CircularWindow : public OpticalElement {
      GLCappedCylinder         m_cylinder;
      CircularWindowProcessor *m_inputProcessor   = nullptr;
      CircularWindowProcessor *m_outputProcessor  = nullptr;
      TranslatedFrame         *m_inputFrame       = nullptr;
      TranslatedFrame         *m_outputFrame      = nullptr;

      Real m_thickness = 1e-2;
      Real m_radius    = 1e-2;
      Real m_mu        = 1.5;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      CircularWindow(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~CircularWindow();

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  class CircularWindowFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _CIRCULAR_WINDOW_H
