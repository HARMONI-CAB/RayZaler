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


#ifndef _RAY_TYPES_H
#define _RAY_TYPES_H

#include <cstdint>
#include <list>
#include <Vector.h>
#include <cassert>

namespace RZ {
  struct RayBeam;

  struct EMMedium;

  struct Ray {
    // Defined by input
    Vec3 origin;
    Vec3 direction;
    Vec3 uEx; // Direction of the Ex vector

    Complex Ex, Ey; // Initial complex amplitudes for the X and Y directions

    // Incremented by tracer
    Real length;
    Real cumOptLength;

    // Defines whether the ray is susceptible to vignetting
    bool chief;
    bool intercepted;

    Real wavelength;
    const EMMedium *medium;

    // Defined by the user
    uint32_t id;

    Ray();
  };

  class RayList : public std::list<RZ::Ray, std::allocator<RZ::Ray>> { };

  struct RayBeamStatistics {
    uint64_t intercepted = 0;
    uint64_t vignetted   = 0;
    uint64_t pruned      = 0;

    inline RayBeamStatistics &
    operator +=(RayBeamStatistics const &existing)
    {
      intercepted += existing.intercepted;
      vignetted   += existing.vignetted;
      pruned      += existing.pruned;

      return *this;
    }
  };

  template<typename T>
  struct Slice {
    T *beam = nullptr;
    uint64_t start = 0;
    uint64_t end   = 0;

    inline uint64_t length() const { return end - start; }
    inline void copyTo(Slice<RayBeam> const &) const; // Definition in RayBeam.h
    inline Slice(T *beam, uint64_t start, uint64_t end);
    inline Slice(T *beam);
    inline Slice();
  };

  template<typename T>
  inline Slice<T>::Slice(T *beam, uint64_t start, uint64_t end) : beam(beam) {
    assert(start <= end);
    assert(end <= beam->count);
    assert(start < beam->count || (start == 0 && beam->count == 0));

    this->start = start;
    this->end   = end;
  }

  template<class T>
  inline Slice<T>::Slice(T *beam) : Slice(beam, 0, beam->count) { }

  template<class T>
  inline Slice<T>::Slice() : beam(nullptr), start(0), end(0) { }

  struct RayBeam;

  typedef Slice<RayBeam>       RayBeamSlice;
  typedef Slice<const RayBeam> ConstRayBeamSlice;
}

#endif // _RAY_TYPES_H
