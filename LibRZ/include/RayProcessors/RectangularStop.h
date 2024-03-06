#ifndef _RAY_PROCESSORS_RECTANGULAR_STOP_H
#define _RAY_PROCESSORS_RECTANGULAR_STOP_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class RectangularStopProcessor : public RayTransferProcessor {
      Real m_width  = .1;
      Real m_height = .1;

    public:
      RectangularStopProcessor();
      void setWidth(Real);
      void setHeight(Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_RECTANGULAR_STOP_H
