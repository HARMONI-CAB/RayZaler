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

#ifndef _MEDIUM_BOUNDARY_H
#define _MEDIUM_BOUNDARY_H

#include "Random.h"

#define RZ_SPEED_OF_LIGHT 299792458 // m/s
#define RZ_WAVELENGTH     555e-9

namespace RZ {
  class SurfaceShape;
  class ReferenceFrame;
  class EMInterface;

  struct RayBeam;
  struct RayBeamSlice;

  class MediumBoundary {
    SurfaceShape   *m_surfaceShape  = nullptr;
    EMInterface    *m_emInterface   = nullptr;
    bool            m_reversible    = false;
    bool            m_complementary = false;

  protected:
    inline void
    setSurfaceShape(SurfaceShape *ap)
    {
      m_surfaceShape = ap;
    }

    inline void
    setEMInterface(EMInterface *em)
    {
      m_emInterface = em;
    }

    inline void
    setReversible(bool rev)
    {
      m_reversible = rev;
    }

    inline void
    setComplementary(bool comp)
    {
      m_complementary = comp;
    }


  public:
    inline bool
    reversible() const
    {
      return m_reversible;
    }
    
    inline bool
    isComplementary() const
    {
      return m_complementary;
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

    inline EMInterface *
    emInterface() const
    {
      return m_emInterface;
    }

    template <class T>
    inline T *
    emInterface()
    {
      return static_cast<T *>(emInterface());
    }
    
    template <class T>
    inline T const *
    emInterface() const
    {
      return static_cast<const T *>(emInterface());
    }

    virtual std::string name() const = 0;
    
    virtual void cast(RayBeam &) const;
    virtual void transmit(RayBeamSlice const &) const;

    virtual ~MediumBoundary();
  };
}

#endif // _MEDIUM_BOUNDARY_H

