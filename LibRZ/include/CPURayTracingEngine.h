#ifndef _CPU_RAYTRACING_ENGINE_H
#define _CPU_RAYTRACING_ENGINE_H

#include "RayTracingEngine.h"

namespace RZ {
  class CPURayTracingEngine : public RayTracingEngine {
    protected:
      virtual void cast(Point3 const &center,  Vec3 const &normal);

    public:
      CPURayTracingEngine();
  };
}

#endif // _CPU_RAYTRACING_ENGINE_H
