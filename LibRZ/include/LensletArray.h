#ifndef _LENSLET_ARRAY_H
#define _LENSLET_ARRAY_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class LensletArray : public OpticalElement {
      GLCappedCylinder        m_cylinder;
      GLSphericalCap          m_cap;
      LensletArrayProcessor  *m_inputProcessor   = nullptr;
      LensletArrayProcessor  *m_outputProcessor  = nullptr;
      TranslatedFrame        *m_inputFrame       = nullptr;
      TranslatedFrame        *m_outputFrame      = nullptr;

      TranslatedFrame        *m_inputFocalPlane  = nullptr;
      TranslatedFrame        *m_outputFocalPlane = nullptr;

      TranslatedFrame        *m_objectPlane      = nullptr;
      TranslatedFrame        *m_imagePlane       = nullptr;

      Real m_thickness = 1e-2;
      Real m_width     = 1e-1;
      Real m_height    = 1e-1;
      unsigned m_rows  = 10;
      unsigned m_cols  = 10;
      Real m_rCurv     = 1;
      Real m_mu        = 1.5;
      Real m_depth     = 0;
      Real m_f         = 0;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      LensletArray(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~LensletArray();

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  class LensletArrayFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _LENSLET_ARRAY_H
