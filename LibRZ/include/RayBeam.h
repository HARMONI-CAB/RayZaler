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


#ifndef _RAY_BEAM_H
#define _RAY_BEAM_H

#include <vector>
#include <map>
#include <functional>

#include "RayTypes.h"
#include "MediumBoundary.h"

#define RZ_BEAM_MINIMUM_WAVELENGTH 1e-12

namespace RZ {
  class ReferenceFrame;
  class OpticalSurface;

  enum RayExtractionMask {
    OriginPOV                  = 1,
    DestinationPOV             = 2,
    BeamIsSurfaceRelative      = 4,
    RayShouldBeSurfaceRelative = 8,
    ExtractIntercepted         = 16,
    ExtractVignetted           = 32,
    ExcludeBeam                = 64,
    ExtractAll                 = ExtractIntercepted | ExtractVignetted
  };

  struct RayBeam {
    uint64_t count         = 0;
    uint64_t allocation    = 0;
    bool nonSeq            = false; // Non sequential beam (allocs surfaces)
    bool fields            = false; // Fields allocated

    Real *origins          = nullptr;
    Real *directions       = nullptr;
    Real *destinations     = nullptr;
    Real *uEx              = nullptr;
    Complex *Ex            = nullptr;
    Complex *Ey            = nullptr;
    Real *lengths          = nullptr;
    Real *cumOptLengths    = nullptr;
    Real *normals          = nullptr; // Surface normals of the boundary surface
    Real *wavelengths      = nullptr;
    const EMMedium **media = nullptr;

    uint32_t *ids          = nullptr;

    uint64_t *mask         = nullptr;
    uint64_t *intMask      = nullptr;
    uint64_t *prevMask     = nullptr;
    uint64_t *chiefMask    = nullptr;
    
    OpticalSurface **surfaces     = nullptr;

    inline bool
    isChief(uint64_t index) const
    {
      return (chiefMask[index >> 6] & (1ull << (index & 63))) >> (index & 63);
    }

    inline bool
    isIntercepted(uint64_t index) const
    {
      return (intMask[index >> 6] & (1ull << (index & 63))) >> (index & 63);
    }

    inline bool
    hasRay(uint64_t index) const
    {
      return (~mask[index >> 6] & (1ull << (index & 63))) >> (index & 63);
    }

    inline void
    prune(uint64_t c)
    {
      if (!isChief(c) && hasRay(c))
        mask[c >> 6] |= 1ull << (c & 63);
    }

    inline void
    pruneAll()
    {
      size_t maskSize = ((count + 63) >> 6) << 3;
      memset(mask, 0xff, maskSize);
    }

    inline void
    uninterceptAll()
    {
      size_t maskSize = ((count + 63) >> 6) << 3;
      memset(intMask, 0, maskSize);
    }

    inline void
    intercept(uint64_t c)
    {
      if (hasRay(c))
        intMask[c >> 6] |= 1ull << (c & 63);
    }

    inline bool
    setChiefRay(uint64_t c)
    {
      if (!hasRay(c))
        return false;

      chiefMask[c >> 6] |= 1ull << (c & 63);
      return true;
    }

    inline bool
    unsetsetChiefRay(uint64_t c)
    {
      if (!hasRay(c))
        return false;

      chiefMask[c >> 6] &= ~(1ull << (c & 63));
      return true;
    }

    inline bool
    hadRay(uint64_t index) const
    {
      return (~prevMask[index >> 6] & (1ull << (index & 63))) >> (index & 63);
    }

    #define SETMASK(field) \
      field[word] = (field[word] & ~bit) | (existing->field[word] & bit)
    inline void
    copyRay(const RayBeam *existing, uint64_t index)
    {
      uint64_t bit  = 1ull << (index & 63);
      uint64_t word = index >> 6;

      memcpy(origins      + 3 * index, existing->origins      + 3 * index, 3 * sizeof(Real));
      memcpy(directions   + 3 * index, existing->directions   + 3 * index, 3 * sizeof(Real));
      memcpy(normals      + 3 * index, existing->normals      + 3 * index, 3 * sizeof(Real));
      memcpy(destinations + 3 * index, existing->destinations + 3 * index, 3 * sizeof(Real));

      lengths[index]       = existing->lengths[index];
      cumOptLengths[index] = existing->cumOptLengths[index];
      media[index]         = existing->media[index];
      wavelengths[index]   = existing->wavelengths[index];
      ids[index]           = existing->ids[index];

      if (fields && existing->fields) {
        Ex[index]            = existing->Ex[index];
        Ey[index]            = existing->Ey[index];
        memcpy(uEx + 3 * index, existing->uEx + 3 * index, 3 * sizeof(Real));
      }

      SETMASK(mask);
      SETMASK(chiefMask);
      SETMASK(intMask);
      SETMASK(prevMask);
    }
    #undef SETMASK

    virtual void allocate(uint64_t);
    virtual void shrink(uint64_t);
    virtual void deallocate();

    template <class T> void extractRays(
      T &dest,
      uint32_t mask,
      OpticalSurface *current = nullptr,
      RayBeamSlice const &beam = RayBeamSlice());

    template <class T> static void extractRays(
      T &dest,
      RayBeamSlice const &slice,
      uint32_t mask,
      OpticalSurface *current = nullptr,
      RayBeamSlice const &beam = RayBeamSlice());


    Real power() const;
    void clearMask();
    void computeInterceptStatistics(OpticalSurface * = nullptr);
    void updateOrigins();

    //
    // toRelative and fromRelative only act on:
    //
    // - origins
    // - destinations
    // - directions
    //
    // we leave normals untouched, as we only care about them during
    // EMInterface calculations
    //
    void copyTo(RayBeam *) const;
    void appendTo(RayBeam *) const;
    void toRelative(const ReferenceFrame *plane);
    void toRelative(RayBeam *, const ReferenceFrame *plane) const;

    void fromRelative(const ReferenceFrame *plane);
    void fromSurfaceRelative();
    void pruneStrayLight();
    void compactify();

    void walk(
      OpticalSurface *,
      const std::function <void (OpticalSurface *, RayBeamSlice const &)>& f,
      const std::function <bool (OpticalSurface *, RayBeam const *, uint64_t)>& include);

    void walk(
      OpticalSurface *,
      const std::function <void (OpticalSurface *, RayBeamSlice const &)>& f);

    void walk(const std::function <void (ConstRayBeamSlice const &)>& f) const;

    uint64_t updateFromVisible(
      const OpticalSurface *currentSurface,
      const RayBeam *beam);
    void debug() const;

    RayBeam(uint64_t, bool surfaces = false, bool field = false);
    ~RayBeam();

  private:
    void addInterceptMetrics(OpticalSurface *surface, RayBeamSlice const &slice);
  };

  template<typename T> inline void
  Slice<T>::copyTo(Slice<RayBeam> const &slice) const
  {
    auto len = length();
    assert(len == slice.length());

    uint64_t sOff   = this->start;
    uint64_t dOff   = slice.start;
    uint64_t sOffV = 3 * sOff;
    uint64_t dOffV = 3 * dOff;

    auto dest = slice.beam;
    auto src  = this->beam;

#define COPYSCALAR(field) \
    memmove(dest->field + dOff, src->field + sOff, len * sizeof(src->field[0]))
  
#define COPYVECTOR(field) \
    memmove(dest->field + dOffV, src->field + sOffV, 3 * len * sizeof(src->field[0]))
  
    COPYSCALAR(lengths);
    COPYSCALAR(cumOptLengths);
    COPYSCALAR(wavelengths);
    COPYSCALAR(media);
    COPYSCALAR(ids);

    if (src->nonSeq && dest->nonSeq)
      COPYSCALAR(surfaces);

    COPYVECTOR(origins);
    COPYVECTOR(destinations);
    COPYVECTOR(directions);

    if (src->fields && dest->fields) {
      COPYSCALAR(Ex);
      COPYSCALAR(Ey);
      COPYVECTOR(uEx);
    }
    
    uint64_t d = dOff;

    // TODO: Optimize.
    
#define COPYMASKBIT(field) \
    dest->field[dBlock] = (dest->field[dBlock] & ~(1ull << dBit)) | (((src->field[sBlock] >> sBit) & 1ull) << dBit)

    for (uint64_t i = this->start; i < this->end; ++i, ++d) {
      uint64_t sBlock = i >> 6;
      uint64_t sBit   = i & 63;

      uint64_t dBlock = d >> 6;
      uint64_t dBit   = d & 63;

      COPYMASKBIT(mask);
      COPYMASKBIT(intMask);
      COPYMASKBIT(prevMask);
      COPYMASKBIT(chiefMask);
    }

#undef COPYMASKBIT
#undef COPYVECTOR
#undef COPYSCALAR
  }
}

#endif // _RAY_BEAM_H

