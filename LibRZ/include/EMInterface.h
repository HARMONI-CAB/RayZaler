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
#include "RayBeam.h"
#include "Random.h"
#include "ReferenceFrame.h"

namespace RZ {
  class ReferenceFrame;

  //
  // EMMedium characterizes the dielectric properties of an electromagnetic
  // medium where waves can propagate.
  //

  enum EMMediumType {
    EMMediumVacuum,     // Ref index is just 1
    EMMediumIsotropic,  // Ref index is > 1
    EMMediumUniaxial    // Two indicies + reference frame + axis

    // No biaxial media so far, but it will come soon.
  };

  struct EMMedium {
    EMMediumType    type = EMMediumVacuum;
    
    union {
      Real            n = 1.;
      struct {
        Real no;
        Real ne;
      };
    };

    ReferenceFrame *frame = nullptr;
    Vec3            axis;

    static const EMMedium *vacuum();

    inline bool
    isotropic() const
    {
      return type == EMMediumVacuum || type == EMMediumIsotropic;
    }

    inline Real
    opd(Real dt, Vec3 const &dir) const
    {
      Vec3 absAxis;
      Vec3 fastPath;
      Real slowComp;
      Real slowPath, fastComponent;
    
      switch (type) {
        case EMMediumVacuum:
          return dt;

        case EMMediumIsotropic:
          return dt * n;

        case EMMediumUniaxial:
          // TODO: Cache stuff somewhere. This is a per-ray operation
          absAxis = frame->fromRelativeVec(axis);
          slowComp  = absAxis * dir;

          fastPath  = no * dt * (dir - slowComp * absAxis);
          slowPath  = ne * dt * slowComp;
          
          return sqrt(slowPath * slowPath + fastPath * fastPath);
      }

      return dt;
    }

    inline void
    advancePhase(
      Complex &Ex,
      Complex &Ey,
      Vec3 const &uEx,
      Vec3 const &dir,
      Real K,
      Real dt) const {
      Complex phiEx = 1, phiEy = 1;

      switch (type) {
        case EMMediumVacuum:
          phiEx = phiEy = std::exp(Complex(0, K * dt));
          break;

        case EMMediumIsotropic:
          phiEx = phiEy = std::exp(Complex(0, n * K * dt));
          break;

        case EMMediumUniaxial:
          break;
      }

      Ex *= phiEx;
      Ey *= phiEy;
    }
  };
  

  //
  // It is important to remark that the EMInterface works in the reference
  // frame of the capture surface. We do not need to convert things back
  // to the absolute reference frames until all transfer took place.
  //
  class EMInterface {
      ExprRandomState          m_randState;
      Real                     m_transmission     = 1.;
      std::vector<Real> const *m_txMap            = nullptr;
      bool                     m_fullyOpaque      = false;
      bool                     m_fullyTransparent = true;
      const EMMedium          *m_pMedium          = nullptr; // Medium in the positive normal
      const EMMedium          *m_nMedium          = nullptr; // Medium in the negative normal
      const EMMedium          *m_surroundings     = nullptr; // Medium if unspecified

      // Only relevant if m_txMap is non-null
      unsigned int             m_cols         = 0;
      unsigned int             m_rows         = 0;
      unsigned int             m_stride       = 0;
      Real                     m_hx           = 0;
      Real                     m_hy           = 0;

    protected:
      inline const EMMedium *
      pMedium() const
      {
        return m_pMedium == nullptr ? m_surroundings : m_pMedium;
      }

      inline const EMMedium *
      nMedium() const
      {
        return m_nMedium == nullptr ? m_surroundings : m_nMedium;
      }

      inline const EMMedium *
      surroundings() const
      {
        return m_surroundings;
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

      static inline void
      snell(Vec3 &u, Vec3 const &normal, Real muIORatio)
      {
        Vec3 nXu  = muIORatio * normal.cross(u);
        Real nXu2 = nXu * nXu;
        
        if (nXu2 < 1)
          u = -normal.cross(nXu) - normal * sqrt(1 - nXu * nXu);
        else
          reflection(u, normal);
      }

      static inline Vec3
      snell(Vec3 const &u, Vec3 const &normal, Real muIORatio)
      {
        Vec3 nXu  = muIORatio * normal.cross(u);
        Real nXu2 = nXu * nXu;
        
        if (nXu2 < 1)
          return -normal.cross(nXu) - normal * sqrt(1 - nXu * nXu);
        else
          return reflection(u, normal);
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

      static inline bool
      mustTransmitRay(const RayBeam *beam, uint64_t i)
      {
        return beam->hasRay(i) && beam->isIntercepted(i);
      }

      void blockLight(RayBeamSlice const &slice);

    public:
      virtual void setSurroundingMedium(const EMMedium *);
      virtual void setMedia(
        const EMMedium *positive = nullptr,
        const EMMedium *negative = nullptr);

      void setTransmission(Real);
      void setTransmission(
        Real width,
        Real height,
        std::vector<Real> const &map,
        unsigned int cols,
        unsigned int rows,
        unsigned int stride);

      virtual std::string name() const = 0;
      virtual void transmit(RayBeamSlice const &beam, RayBeam *splinterRays) = 0;
      virtual ~EMInterface();
  };
}

#endif // _EM_INTERFACE_H
