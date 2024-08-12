#ifndef _RAY_PROCESSORS_FLAT_MIRROR_H
#define _RAY_PROCESSORS_FLAT_MIRROR_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class FlatMirrorProcessor : public RayTransferProcessor {
      Real m_radius = .5;
      Real m_ecc    = 0;

    public:
      FlatMirrorProcessor();
      void setRadius(Real);
      void setEccentricity(Real ecc);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_FLAT_MIRROR
