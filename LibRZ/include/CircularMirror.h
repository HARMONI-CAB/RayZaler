#ifndef _CIRCULAR_MIRROR_H
#define _CIRCULAR_MIRROR_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class CircularMirror : public OpticalElement {
      GLCappedCylinder m_cylinder;
      CircularMirrorProcessor *m_processor;
      TranslatedFrame *m_reflectiveSurfaceFrame = nullptr;
      Real m_thickness = 1e-2;
      Real m_radius = 1e-2;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      CircularMirror(std::string const &, ReferenceFrame *, Element *parent = nullptr);
      ~CircularMirror();

      virtual void renderOpenGL() override;
  };

  class CircularMirrorFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _CIRCULAR_MIRROR_H
