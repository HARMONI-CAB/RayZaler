#ifndef _FLAT_MIRROR_H
#define _FLAT_MIRROR_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class FlatMirror : public OpticalElement {
      GLCappedCylinder m_cylinder;
      FlatMirrorProcessor *m_processor;
      TranslatedFrame *m_reflectiveSurfaceFrame = nullptr;
      Real m_thickness = 1e-2;
      Real m_radius = 1e-2;
      Real m_width  = 2e-2;
      Real m_height = 2e-2;
      Real m_a      = 1;
      Real m_b      = 1;
      
      // Calculated members
      Real m_ecc    = 0;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      FlatMirror(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~FlatMirror();

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  class FlatMirrorFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _FLAT_MIRROR_H
