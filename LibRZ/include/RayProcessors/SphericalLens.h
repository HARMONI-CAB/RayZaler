#ifndef _RAY_PROCESSORS_SPHERICAL_LENS_H
#define _RAY_PROCESSORS_SPHERICAL_LENS_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class SphericalLensProcessor : public RayTransferProcessor {
      Real m_radius = .5;
      Real m_rCurv  =  1;
      Real m_muOut  = 1.5;
      Real m_muIn   = 1;
      Real m_IOratio = 1 / 1.5;
      
      Real m_center = .866;
      bool m_convex = true;

      void recalcCurvCenter();

    public:
      SphericalLensProcessor();
      
      void setConvex(bool);
      void setRadius(Real);
      void setCurvatureRadius(Real);
      void setRefractiveIndex(Real , Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_SPHERICAL_LENS_H
