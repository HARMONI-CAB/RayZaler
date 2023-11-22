#ifndef _RAYPROCESSORS_H
#define _RAYPROCESSORS_H

#include <RayTracingEngine.h>
#include <vector>
#include "Zernike.h"

#define RZ_OUTBOUND_RAY_LENGTH 10

namespace RZ {
  class ReferenceFrame;
  class PassThroughProcessor : public RayTransferProcessor {
  public:
    virtual std::string name() const;
    virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  class InfiniteMirrorProcessor : public RayTransferProcessor {
  public:
    virtual std::string name() const;
    virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  class FlatMirrorProcessor : public RayTransferProcessor {
      Real m_radius = .5;
    public:
      void setRadius(Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  class SphericalMirrorProcessor : public RayTransferProcessor {
      Real m_radius = .5;
      Real m_flength = 1;

    public:
      void setRadius(Real);
      void setFocalLength(Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  class ParabolicMirrorProcessor : public RayTransferProcessor {
      Real m_radius = .5;
      Real m_flength = 1;

    public:
      void setRadius(Real);
      void setFocalLength(Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  class SquareFlatSurfaceProcessor: public RayTransferProcessor {
      Real m_width  = .1;
      Real m_height = .1;
      Real m_muOut  = 1.5;
      Real m_muIn   = 1;
      Real m_IOratio = 1 / 1.5;

    public:
      void setWidth(Real);
      void setHeight(Real);
      void setRefractiveIndex(Real , Real);
      virtual std::string name() const override;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  class SphericalLensProcessor : public RayTransferProcessor {
      Real m_radius = .5;
      Real m_rCurv  =  1;
      Real m_muOut  = 1.5;
      Real m_muIn   = 1;
      Real m_IOratio = 1 / 1.5;
      
      Real m_center = .866;
      bool m_convex = true;

      void recalcCurvCenter();

    public:
      void setConvex(bool);
      void setRadius(Real);
      void setCurvatureRadius(Real);
      void setRefractiveIndex(Real , Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  
  class LensletArrayProcessor : public RayTransferProcessor {
      Real m_width         = 100e-3;
      Real m_height        = 100e-3;
      Real m_lensletWidth  = 10e-3;
      Real m_lensletHeight = 10e-3;
      Real m_lensletRadius;
      Real m_rCurv         = 1;
      Real m_muOut         = 1.5;
      Real m_muIn          = 1;
      Real m_IOratio       = 1 / 1.5;
      Real m_convex        = true;
      Real m_center        = .866;

      unsigned int m_cols          = 10; // X dimension
      unsigned int m_rows          = 10; // Y dimension
      bool         m_dirty         = true;

      void recalculateDimensions();

    public:
      Real
      lensletRadius() const {
        return m_lensletRadius;
      }

      LensletArrayProcessor();
      void setCurvatureRadius(Real);
      void setRefractiveIndex(Real, Real);
      void setConvex(bool);
      void setWidth(Real);
      void setHeight(Real);
      void setCols(unsigned);
      void setRows(unsigned);
      virtual std::string name() const override;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  class RectangularStopProcessor : public RayTransferProcessor {
      Real m_width  = .1;
      Real m_height = .1;

    public:
      void setWidth(Real);
      void setHeight(Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  class ApertureStopProcessor : public RayTransferProcessor {
      Real m_radius = .5;

    public:
      void setRadius(Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  class PhaseScreenProcessor : public RayTransferProcessor {
      Real m_radius        = .5;
      std::vector<Zernike> m_poly;
      std::vector<Real>    m_coef;
      Real m_muOut         = 1.5;
      Real m_muIn          = 1;
      Real m_IOratio       = 1 / 1.5;

      Real dZdx(Real x, Real y) const;
      Real dZdy(Real x, Real y) const;

    public:
      Real Z(Real x, Real y) const;
      void setRadius(Real);
      void setCoef(unsigned int ansi, Real value);
      void setRefractiveIndex(Real, Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  class ObstructionProcessor : public RayTransferProcessor {
      Real m_radius = .5;

    public:
      void setRadius(Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  void registerRayProcessors();
}

#endif // _RAYPROCESSORS_H
