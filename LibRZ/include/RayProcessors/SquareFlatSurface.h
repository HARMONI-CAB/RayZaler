#ifndef _RAY_PROCESSORS_SQUARE_FLAT_SURFACE_H
#define _RAY_PROCESSORS_SQUARE_FLAT_SURFACE_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class SquareFlatSurfaceProcessor: public RayTransferProcessor {
      Real m_width  = .1;
      Real m_height = .1;
      Real m_muOut  = 1.5;
      Real m_muIn   = 1;
      Real m_IOratio = 1 / 1.5;

    public:
      void setWidth(Real);
      void setHeight(Real);
      void setRefractiveIndex(Real , Real);
      virtual std::string name() const override;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_SQUARE_FLAT_SURFACE_H
