#ifndef _RAY_PROCESSORS_APERTURE_STOP_H
#define _RAY_PROCESSORS_APERTURE_STOP_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class ApertureStopProcessor : public RayTransferProcessor {
      Real m_radius = .5;

    public:
      ApertureStopProcessor();
      void setRadius(Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_APERTURE_STOP_H
