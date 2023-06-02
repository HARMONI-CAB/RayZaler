#ifndef _Concave_MIRROR_H
#define _Concave_MIRROR_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class ConcaveMirror : public OpticalElement {
      GLCappedCylinder m_cylinder;
      SphericalMirrorProcessor *m_processor;
      TranslatedFrame *m_reflectiveSurfaceFrame = nullptr;
      Real m_thickness = 1e-2;
      Real m_radius = 1e-2;
      Real m_flength = 1;
      
      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      ConcaveMirror(std::string const &, ReferenceFrame *, Element *parent = nullptr);
      ~ConcaveMirror();

      virtual void renderOpenGL() override;
  };

  class ConcaveMirrorFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _Concave_MIRROR_H
