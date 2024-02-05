#ifndef _RAY_PROCESSORS_INFINITE_MIRROR_H
#define _RAY_PROCESSORS_INFINITE_MIRROR_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class InfiniteMirrorProcessor : public RayTransferProcessor {
  public:
    virtual std::string name() const;
    virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_INFINITE_MIRROR_H
