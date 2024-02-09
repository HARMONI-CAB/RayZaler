#ifndef _RAY_PROCESSORS_LENSLET_ARRAY_H
#define _RAY_PROCESSORS_LENSLET_ARRAY_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class LensletArrayProcessor : public RayTransferProcessor {
      Real m_lensletRadius;
      Real m_rCurv         = 1;
      Real m_muOut         = 1.5;
      Real m_muIn          = 1;
      Real m_IOratio       = 1 / 1.5;
      Real m_convex        = true;
      Real m_center        = .866;

      bool         m_dirty         = true;

      void recalculateDimensions();

    public:
      Real
      lensletRadius() const
      {
        return m_lensletRadius;
      }

      LensletArrayProcessor();
      void setCurvatureRadius(Real);
      void setRefractiveIndex(Real, Real);
      void setConvex(bool);
      void setWidth(Real);
      void setHeight(Real);
      void setCols(unsigned);
      void setRows(unsigned);
      virtual std::string name() const override;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_LENSLET_ARRAY_H
