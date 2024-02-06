#ifndef _RAY_PROCESSORS_PARABOLIC_MIRROR_H
#define _RAY_PROCESSORS_PARABOLIC_MIRROR_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class ParabolicMirrorProcessor : public RayTransferProcessor {
      Real m_radius = .5;
      Real m_flength = 1;

    public:
      ParabolicMirrorProcessor();
      void setRadius(Real);
      void setFocalLength(Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_PARABOLIC_MIRROR_H
