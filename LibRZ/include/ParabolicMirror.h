#ifndef _PARABOLIC_MIRROR_H
#define _PARABOLIC_MIRROR_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class ParabolicMirror : public OpticalElement {
      GLCappedCylinder m_cylinder;
      GLParabolicCap   m_cap, m_rearCap;
      ParabolicMirrorProcessor *m_processor;
      TranslatedFrame *m_reflectiveSurfaceFrame = nullptr;
      TranslatedFrame *m_reflectiveSurfacePort = nullptr;
      Real m_thickness = 1e-2;
      Real m_radius = 1e-2;
      Real m_flength = 1;
      Real m_displacement;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      ParabolicMirror(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~ParabolicMirror();

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  class ParabolicMirrorFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _PARABOLIC_MIRROR_H
