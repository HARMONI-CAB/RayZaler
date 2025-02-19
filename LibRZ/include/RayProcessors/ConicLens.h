//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

#ifndef _RAY_PROCESSORS_CONIC_LENS_H
#define _RAY_PROCESSORS_CONIC_LENS_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class ConicLensProcessor : public RayTransferProcessor {
      Real m_radius  = 0.5;
      Real m_K       = 0;
      Real m_rCurv   = 1;
      Real m_x0      = 0;
      Real m_y0      = 0;

      Real m_muOut   = 1.5;
      Real m_muIn    = 1;
      Real m_IOratio = 1 / 1.5;

      bool m_convex  = false;

    public:
      ConicLensProcessor();
      virtual ~ConicLensProcessor() = default;
      void setRadius(Real);
      void setCenterOffset(Real, Real);

      void setCurvatureRadius(Real);
      void setConicConstant(Real);
      void setRefractiveIndex(Real , Real);
      void setConvex(bool);

      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_CONIC_MIRROR_H
