#ifndef _RAY_PROCESSORS_PASS_THROUGH
#define _RAY_PROCESSORS_PASS_THROUGH

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;
  class PassThroughProcessor : public RayTransferProcessor {
  public:
    virtual std::string name() const;
    virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}


#endif // _RAY_PROCESSORS_PASS_THROUGH
