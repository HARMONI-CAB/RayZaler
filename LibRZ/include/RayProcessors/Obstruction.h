#ifndef _RAY_PROCESSORS_OBSTRUCTION_H
#define _RAY_PROCESSORS_OBSTRUCTION_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class ObstructionProcessor : public RayTransferProcessor {
      Real m_radius = .5;

    public:
      void setRadius(Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_OBSTRUCTION_H
