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

  void registerRayProcessors();
}

#endif // _RAYPROCESSORS_H
