#ifndef _RAY_PROCESSORS_SPHERICAL_MIRROR
#define _RAY_PROCESSORS_SPHERICAL_MIRROR

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class SphericalMirrorProcessor : public RayTransferProcessor {
      Real m_radius = .5;
      Real m_flength = 1;

    public:
      SphericalMirrorProcessor();
      void setRadius(Real);
      void setFocalLength(Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_SPHERICAL_MIRROR
