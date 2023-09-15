#ifndef _RAYPROCESSORS_H
#define _RAYPROCESSORS_H

#include <RayTracingEngine.h>

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

  class ApertureStopProcessor : public RayTransferProcessor {
      Real m_radius = .5;

    public:
      void setRadius(Real);
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
