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

#ifndef _MEDIUM_H
#define _MEDIUM_H

#include "Random.h"

#define RZ_SPEED_OF_LIGHT 299792458 // m/s
#define RZ_WAVELENGTH     555e-9

namespace RZ {
  class  SurfaceShape;
  class ReferenceFrame;
  struct RayBeam;

  class RayTransferProcessor {
    SurfaceShape   *m_surfaceShape = nullptr;
    ExprRandomState m_randState;
    bool            m_reversible = false;

  protected:
    inline void
    setSurfaceShape(SurfaceShape *ap)
    {
      m_surfaceShape = ap;
    }

    inline void
    setReversible(bool rev)
    {
      m_reversible = rev;
    }

  public:
    inline bool
    reversible() const
    {
      return m_reversible;
    }
    
    inline SurfaceShape *
    surfaceShape() const
    {
      return m_surfaceShape;
    }

    template <class T>
    inline T *
    surfaceShape()
    {
      return static_cast<T *>(surfaceShape());
    }
    
    template <class T>
    inline T const *
    surfaceShape() const
    {
      return static_cast<const T *>(surfaceShape());
    }

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

    virtual std::string name() const = 0;
    virtual void process(RayBeam &, const ReferenceFrame *) const = 0;
    virtual ~RayTransferProcessor();
  };
}

#endif // _MEDIUM_H

