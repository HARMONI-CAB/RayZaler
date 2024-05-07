#ifndef _RAY_PROCESSORS_CIRCULAR_WINDOW_H
#define _RAY_PROCESSORS_CIRCULAR_WINDOW_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class CircularWindowProcessor : public RayTransferProcessor {
      Real m_radius = .5;
      Real m_muOut  = 1.5;
      Real m_muIn   = 1;
      Real m_IOratio = 1 / 1.5;

    public:
      CircularWindowProcessor();
      
      void setRadius(Real);
      void setRefractiveIndex(Real , Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_CIRCULAR_WINDOW_H
