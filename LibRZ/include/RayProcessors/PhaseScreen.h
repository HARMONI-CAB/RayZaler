#ifndef _RAY_PROCESSORS_PHASE_SCREEN_H
#define _RAY_PROCESSORS_PHASE_SCREEN_H

#include <RayTracingEngine.h>
#include "Zernike.h"

namespace RZ {
  class ReferenceFrame;

  class PhaseScreenProcessor : public RayTransferProcessor {
      Real m_radius        = .5;
      std::vector<Zernike> m_poly;
      std::vector<Real>    m_coef;
      Real m_muOut         = 1.5;
      Real m_muIn          = 1;
      Real m_IOratio       = 1 / 1.5;

      Real dZdx(Real x, Real y) const;
      Real dZdy(Real x, Real y) const;

    public:
      PhaseScreenProcessor();
      
      inline Real
      coef(unsigned int ansi) const
      {
        if (ansi >= m_coef.size())
          return 0;
          
        return m_coef[ansi];
      }

      Real Z(Real x, Real y) const;
      void setRadius(Real);
      void setCoef(unsigned int ansi, Real value);
      void setRefractiveIndex(Real, Real);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_PHASE_SCREEN_H
