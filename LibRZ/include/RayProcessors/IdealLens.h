#ifndef _RAY_PROCESSORS_IDEAL_LENS_H
#define _RAY_PROCESSORS_IDEAL_LENS_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class IdealLensProcessor : public RayTransferProcessor {
      Real m_radius = .5;
      Real m_fLen   = 1;

    public:
      IdealLensProcessor();
      
      void setRadius(Real);
      void setFocalLength(Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_CIRCULAR_WINDOW_H
