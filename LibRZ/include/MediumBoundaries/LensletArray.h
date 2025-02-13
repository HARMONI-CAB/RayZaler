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
      Real m_K             = 0;

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
      void setConicConstant(Real);
      void setHeight(Real);
      void setCols(unsigned);
      void setRows(unsigned);
      virtual std::string name() const override;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };
}

#endif // _RAY_PROCESSORS_LENSLET_ARRAY_H
