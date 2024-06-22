#ifndef _CONVEX_LENS_H
#define _CONVEX_LENS_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class ConvexLens : public OpticalElement {
      GLCappedCylinder        m_cylinder;
      GLSphericalCap          m_topCap, m_bottomCap;
      SphericalLensProcessor *m_inputProcessor   = nullptr;
      SphericalLensProcessor *m_outputProcessor  = nullptr;
      TranslatedFrame        *m_inputFrame       = nullptr;
      TranslatedFrame        *m_outputFrame      = nullptr;

      TranslatedFrame        *m_inputFocalPlane  = nullptr;
      TranslatedFrame        *m_outputFocalPlane = nullptr;

      TranslatedFrame        *m_objectPlane      = nullptr;
      TranslatedFrame        *m_imagePlane       = nullptr;

      Real m_thickness = 1e-2;
      Real m_radius    = 1e-2;
      Real m_rCurv     = 1;
      Real m_mu        = 1.5;
      Real m_depth     = 0;
      Real m_f         = 0;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      ConvexLens(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~ConvexLens();

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  class ConvexLensFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _CONVEX_LENS_H
