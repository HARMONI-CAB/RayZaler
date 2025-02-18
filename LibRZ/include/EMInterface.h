//
//  Copyright (c) 2025 Gonzalo José Carracedo Carballal
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

#ifndef _EM_INTERFACE_H
#define _EM_INTERFACE_H

#include <string>
#include "Random.h"

namespace RZ {
  class RayBeam;
  class ReferenceFrame;

  //
  // It is important to remark that the EMInterface works in the reference
  // frame of the capture surface. We do not need to convert things back
  // to the absolute reference frames until all transfer took place.
  //
  class EMInterface {
      ExprRandomState m_randState;
      Real                     m_transmission     = 1.;
      std::vector<Real> const *m_txMap            = nullptr;
      bool                     m_fullyOpaque      = false;
      bool                     m_fullyTransparent = true;

      // Only relevant if m_txMap is non-null
      unsigned int             m_cols         = 0;
      unsigned int             m_rows         = 0;
      unsigned int             m_stride       = 0;
      Real                     m_hx           = 0;
      Real                     m_hy           = 0;

    protected:
      static inline void
      snell(Vec3 &u, Vec3 const &normal, Real muIORatio)
      {
        Vec3 nXu = muIORatio * normal.cross(u);
        u        = -normal.cross(nXu) - normal * sqrt(1 - nXu * nXu);
      }

      static inline Vec3
      snell(Vec3 const &u, Vec3 const &normal, Real muIORatio)
      {
        Vec3 nXu = muIORatio * normal.cross(u);

        return -normal.cross(nXu) - normal * sqrt(1 - nXu * nXu);
      }

      static inline void
      reflection(Vec3 &u, Vec3 const &normal)
      {
        u -= 2 * (u * normal) * normal;
      }

      static inline Vec3
      reflection(Vec3 const &u, Vec3 const &normal)
      {
        return u - 2 * (u * normal) * normal;
      }

      inline ExprRandomState const &
      constRandState() const
      {
        return m_randState;
      }

      inline ExprRandomState &
      randState() const
      {
        return const_cast<ExprRandomState &>(m_randState);
      }

      void blockLight(RayBeam &);

    public:
      void setTransmission(Real);
      void setTransmission(
        Real width,
        Real height,
        std::vector<Real> const &map,
        unsigned int cols,
        unsigned int rows,
        unsigned int stride);

      virtual std::string name() const = 0;
      virtual void transmit(RayBeam &beam) = 0;
      virtual ~EMInterface();
  };
}

#endif // _EM_INTERFACE_H
