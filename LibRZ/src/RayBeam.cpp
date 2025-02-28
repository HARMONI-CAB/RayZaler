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

#include <cassert>

#include "RayBeam.h"
#include <ReferenceFrame.h>
#include <OpticalElement.h>

using namespace RZ;


//
// Under the refactored raytracing abstraction, ray beams go through several
// states:
//
//
//  AIMED:
//    - Rays are available / pruned
//    - Reference system is *absolute*
//    - If *available*:
//      - Origins defined
//      - Directions defined
//      - Will be processed by castTo()
//

//  CASTED:
//    - Same as aimed, but available rays can be either intercepted or vignetted
//    - Reference system is *relative*
//    - If *intercepted*:
//      - Destinations defined
//      - Target surface defined (in nonseq model)
//      - Length defined
//      - Optical length defined
//      - Will be processed by transmitThroughSurface()

//  TRANSMITTED:
//    - Intercepted rays may transition or not to pruned, according to opacity
//    - Reference system is *absolute*
//    - If *NOT pruned*:
//      - New directions updated
//      - Destinations become origins by means of updateOrigins()
//

//
// When we extract intermediate rays, we do so right after CASTED. This means
// that rays may be intercepted or not. Additionally, rays that are not
// intercepted may be intercepted later on. This implies that we must
// save intercepted rays in each stage, and save non-intercepted rays right
// before leaving.
//

//
// There are 4 use cases for ray extraction:
//    - Calculation of surface statistics (intermediate stages)
//      Trivial. Rays are already in relative coordinates. Extraction must
//      be restricted to existing rays. Calculation of vignetted rays only
//      applies to **sequential mode**.
//        @origins:    relative hit locations
//        @directions: arrival directions 
//
//    - Intermediate ray extraction (intermediate stages)
//      Non trivial. Rays must be converted to the appropriate reference
//      system. Rays are restricted to the intercepted ones.
//        @origins:    origins of the rays
//        @directions: directions of the rays
//        @length:     physical length of the ray
//
//    - Non-intercepted intermediate ray extraction (final stage)
//      Trivial. Rays are already in absolute coordinates, and must be restricted
//      to the non-intercepted ones.
//
//    - Extraction of remaining rays (final stage):
//      Trivial. Used by getRays(). All non-pruned rays.
//

template<typename T>
static T *
allocBuffer(uint64_t count, uint64_t prev = 0, T *existing = nullptr)
{
  if (count == 0)
    return nullptr;
  
  T *result = static_cast<T *>(realloc(existing, count * sizeof(T)));

  if (result == nullptr)
    throw std::bad_alloc();

  memset(result + prev, 0, (count - prev) * sizeof(T));
  
  return result;
}

template<typename T>
void
freeBuffer(T *&buf)
{
  if (buf != nullptr) {
    free(buf);
    buf = nullptr;
  }
}

template static Real *allocBuffer<Real>(uint64_t, uint64_t, Real *);
template static void  freeBuffer<Real>(Real *&);

template static uint64_t *allocBuffer<uint64_t>(uint64_t, uint64_t, uint64_t *);
template static void freeBuffer<uint64_t>(uint64_t *&);

void
RayBeam::clearMask()
{
  memset(mask, 0, ((count + 63) >> 6) << 3);
  memset(prevMask, 0, ((count + 63) >> 6) << 3);
}

template <class C> void
RayBeam::extractRays(
      C &dest,
      RayBeamSlice const &slice,
      uint32_t mask,
      OpticalSurface *surface)
{
  bool originPOV             = (mask & OriginPOV) != 0;
  bool destinationPOV        = (mask & DestinationPOV) != 0;
  bool beamIsSurfaceRelative = (mask & BeamIsSurfaceRelative) != 0;
  bool rayIsSurfaceRelative  = (mask & RayShouldBeSurfaceRelative) != 0;
  bool extractIntercepted    = (mask & ExtractIntercepted) != 0;
  bool extractVignetted      = (mask & ExtractVignetted) != 0;

  auto beam = slice.beam;

  assert(originPOV != destinationPOV);
  assert(extractIntercepted || extractVignetted);
  assert(
       beamIsSurfaceRelative == rayIsSurfaceRelative
    || beam->nonSeq
    || surface != nullptr);
  
  for (auto i = slice.start; i < slice.end; ++i) {
    if (beam->hasRay(i) && beam->lengths[i] >= 0) {
      bool shouldExtract = 
        (beam->isIntercepted(i) && extractIntercepted)
        || (!beam->isIntercepted(i) && extractVignetted);

      if (shouldExtract) {
        Ray ray;

        ray.id           = beam->ids[i];
        ray.chief        = beam->isChief(i);
        ray.wavelength   = beam->wavelengths[i];
        ray.refNdx       = beam->refNdx[i];
        ray.cumOptLength = beam->cumOptLengths[i];
        ray.length       = beam->lengths[i];
        ray.direction    = Vec3(beam->directions + 3 * i);
        ray.intercepted  = beam->isIntercepted(i);

        ray.origin       = originPOV 
          ? Vec3(beam->origins + 3 * i) 
          : Vec3(beam->destinations + 3 * i);
        
        if (beamIsSurfaceRelative != rayIsSurfaceRelative) {
          if (beam->nonSeq) {
            surface = beam->surfaces[i];
            
            if (ray.intercepted)
              assert(surface != nullptr);
          }

          if (surface != nullptr) {
            auto plane = surface->frame;

            if (beamIsSurfaceRelative) {
              ray.origin    = plane->fromRelative(ray.origin);
              ray.direction = plane->fromRelativeVec(ray.direction);
            } else {
              ray.origin    = plane->toRelative(ray.origin);
              ray.direction = plane->toRelativeVec(ray.direction);
            }
          }
        }

        dest.push_back(std::move(ray));
      }
    }
  }
}

template <class C> void
RayBeam::extractRays(C &dest, uint32_t mask, OpticalSurface *surf)
{
  extractRays(dest, RayBeamSlice(this), mask, surf);
}

void
RayBeam::addInterceptMetrics(OpticalSurface *surface, RayBeamSlice const &slice)
{
  for (auto i = slice.start; i < slice.end; ++i) {
    if (hasRay(i) && !isChief(i)) {
      if (isIntercepted(i))
        ++surface->statistics[ids[i]].intercepted;
      else
        ++surface->statistics[ids[i]].vignetted;
    } else if (!hasRay(i)) {
      ++surface->statistics[ids[i]].pruned;
    }
  }
}

void
RayBeam::updateOrigins()
{
  auto maskSize = ((count + 63) >> 6) << 3;

  memcpy(origins, destinations, 3 * count * sizeof(Real));
  memcpy(prevMask, mask, maskSize);
}

void
RayBeam::computeInterceptStatistics(OpticalSurface *surface)
{
  walk(
    surface,
    [] (OpticalSurface *surf, RayBeamSlice const &slice) {
      slice.beam->addInterceptMetrics(surf, slice);

      if (surf->parent->recordHits()) {
        extractRays(
          surf->hits,
          slice,
            DestinationPOV
          | BeamIsSurfaceRelative
          | RayShouldBeSurfaceRelative
          | ExtractIntercepted,
          surf);
      }
    });
}

void
RayBeam::toRelative(RayBeam *dest, const ReferenceFrame *plane) const
{
  assert(count == dest->count);
  size_t maskLen = ((count + 63) >> 6) << 3;

  memcpy(dest->mask, mask, maskLen);
  memcpy(dest->prevMask, prevMask, maskLen);
  memcpy(dest->intMask, intMask, maskLen);
  memcpy(dest->chiefMask, chiefMask, maskLen);

  for (uint64_t i = 0; i < this->count; ++i) {
    if (hasRay(i)) {
      plane->toRelative(
        Vec3(origins + 3 * i)).copyToArray(dest->origins + 3 * i);

      plane->toRelative(
        Vec3(destinations + 3 * i)).copyToArray(dest->destinations + 3 * i);

      plane->toRelativeVec(
        Vec3(directions + 3 * i)).copyToArray(dest->directions + 3 * i);

      dest->lengths[i] = lengths[i];
      dest->amplitude[i] = amplitude[i];
      dest->cumOptLengths[i] = cumOptLengths[i];
      dest->wavelengths[i] = wavelengths[i];
      dest->ids[i] = ids[i];
      dest->refNdx[i] = refNdx[i];
    }
  }
}

void
RayBeam::toRelative(const ReferenceFrame *plane)
{
  assert(!this->nonSeq);
  toRelative(this, plane);
}

void
RayBeam::fromRelative(const ReferenceFrame *plane)
{
  assert(!this->nonSeq);

  for (uint64_t i = 0; i < this->count; ++i) {
    if (hasRay(i)) {
      plane->fromRelative(
        Vec3(origins + 3 * i)).copyToArray(origins + 3 * i);

      plane->fromRelative(
        Vec3(destinations + 3 * i)).copyToArray(destinations + 3 * i);

      plane->fromRelativeVec(
        Vec3(directions + 3 * i)).copyToArray(directions + 3 * i);
    }
  }
}

void
RayBeam::fromSurfaceRelative()
{
  assert(this->nonSeq);

  uint64_t total = 0;

  for (uint64_t i = 0; i < this->count; ++i) {
    if (hasRay(i) && this->surfaces[i] != nullptr) {
      printf(
        "%s: %s (length %g)\n",
        this->surfaces[i]->parent->name().c_str(),
        Vec3(directions + 3 * i).toString().c_str(),
        this->lengths[i]);
      auto plane = this->surfaces[i]->frame;

      plane->fromRelative(
        Vec3(origins + 3 * i)).copyToArray(origins + 3 * i);

      plane->fromRelative(
        Vec3(destinations + 3 * i)).copyToArray(destinations + 3 * i);

      plane->fromRelativeVec(
        Vec3(directions + 3 * i)).copyToArray(directions + 3 * i);

      this->surfaces[i] = nullptr;
      ++total;
    }
  }

  printf("fromSurfaceRelative(): %d rays in abs system\n", total);
}

uint64_t
RayBeam::updateFromVisible(const OpticalSurface *surface, const RayBeam *beam)
{
  uint64_t i;
  uint64_t newTransferred = 0;

  assert(nonSeq);
  assert(this->surfaces != nullptr);
  assert(this->count == beam->count);

  printf("Updating beam from visible (%s)\n", surface->parent->name().c_str());

  for (i = 0; i < beam->count; ++i) {
    // Only update NS beam from existing rays
    if (beam->hasRay(i) && beam->isIntercepted(i) && beam->lengths[i] > 0) {
      bool copyRay;

      if (!this->hasRay(i)) {
        ++newTransferred;
        copyRay = true;
      } else {
        copyRay = beam->lengths[i] < this->lengths[i];
      }

      if (copyRay) {
        this->copyRay(beam, i);
        this->surfaces[i] = const_cast<OpticalSurface *>(surface);
      }
    }
  }

  if (newTransferred > 0)
    printf("  New transferred: %d\n", newTransferred);

  return newTransferred;
}

void
RayBeam::allocate(uint64_t count)
{
  size_t maskLen = (count + 63) >> 6;
  size_t prev = this->count;
  size_t prevMaskLen = (this->count + 63) >> 6;

  if (prev == 0) {
    this->origins       = allocBuffer<Real>(3 * count);
    this->directions    = allocBuffer<Real>(3 * count);
    this->normals       = allocBuffer<Real>(3 * count);
    this->destinations  = allocBuffer<Real>(3 * count);
    this->amplitude     = allocBuffer<Complex>(count);
    this->lengths       = allocBuffer<Real>(count);
    this->cumOptLengths = allocBuffer<Real>(count);
    this->refNdx        = allocBuffer<Real>(count);
    this->wavelengths   = allocBuffer<Real>(count);
    this->ids           = allocBuffer<uint32_t>(count);
    this->mask          = allocBuffer<uint64_t>(maskLen);
    this->prevMask      = allocBuffer<uint64_t>(maskLen);
    this->intMask       = allocBuffer<uint64_t>(maskLen);
    this->chiefMask     = allocBuffer<uint64_t>(maskLen);
    
    if (this->nonSeq)
      this->surfaces  = allocBuffer<OpticalSurface *>(count);
    
    this->allocation    = count;
  } else if (count >= this->count) {
    this->origins       = allocBuffer<Real>(3 * count, 3 * prev, this->origins);
    this->directions    = allocBuffer<Real>(3 * count, 3 * prev, this->directions);
    this->normals       = allocBuffer<Real>(3 * count, 3 * prev, this->normals);
    this->destinations  = allocBuffer<Real>(3 * count, 3 * prev, this->destinations);
    this->amplitude     = allocBuffer<Complex>(count, prev, this->amplitude);
    this->wavelengths   = allocBuffer<Real>(count, prev, this->wavelengths);
    this->lengths       = allocBuffer<Real>(count, prev, this->lengths);
    this->cumOptLengths = allocBuffer<Real>(count, prev, this->cumOptLengths);
    this->refNdx        = allocBuffer<Real>(count, prev, this->refNdx);
    this->ids           = allocBuffer<uint32_t>(count, prev, this->ids);
    this->mask          = allocBuffer<uint64_t>(maskLen, prevMaskLen, this->mask);
    this->prevMask      = allocBuffer<uint64_t>(maskLen, prevMaskLen, this->prevMask);
    this->intMask       = allocBuffer<uint64_t>(maskLen, prevMaskLen, this->intMask);
    this->chiefMask     = allocBuffer<uint64_t>(maskLen, prevMaskLen, this->chiefMask);

    if (this->nonSeq)
      this->surfaces  = allocBuffer<OpticalSurface *>(count, prev, this->surfaces);

    this->allocation    = count;
  } else {
    throw std::runtime_error("Cannot shrink ray list");
  }

  memset(
    this->chiefMask + prevMaskLen,
    0,
    (maskLen - prevMaskLen) * sizeof(uint64_t));
  
  for (int64_t i = this->count; i < count; ++i)
    this->refNdx[i] = 1.;

  this->count = count;
}

void
RayBeam::walk(
      OpticalSurface *surface,
      const std::function <void (OpticalSurface *, RayBeamSlice const &)>& func)
{
  auto slice = RayBeamSlice(this); // Start at 0

  if (surface != nullptr) {
    assert(!nonSeq);
    func(surface, slice);
  } else {
    uint64_t i = 0;
    assert(nonSeq);
    
    for (i = 0; i < count; ++i) {
      auto currSurf = hasRay(i) ? surfaces[i] : nullptr;

      if (surface != currSurf) {
        // Sequence of equal surfaces has finished. Transmit this slice.
        if (surface != nullptr) {
          slice.end = i;
          func(surface, slice);
        }

        surface = currSurf;

        slice.start = i;
      }
    }

    if (surface != nullptr) {
      slice.end = count;
      func(surface, slice);
    }
  }
}

void
RayBeam::deallocate()
{
  freeBuffer(origins);
  freeBuffer(directions);
  freeBuffer(destinations);
  freeBuffer(normals);
  freeBuffer(lengths);
  freeBuffer(wavelengths);
  freeBuffer(cumOptLengths);
  freeBuffer(amplitude);
  freeBuffer(ids);
  freeBuffer(mask);
  freeBuffer(prevMask);
  freeBuffer(chiefMask);
  freeBuffer(surfaces);

  this->count = 0;
}

RayBeam::RayBeam(uint64_t count, bool nonSeq)
{
  this->nonSeq = nonSeq;

  allocate(count);

  if (nonSeq)
    pruneAll();
}

RayBeam::~RayBeam()
{
  deallocate();
}

// Explicit instantiations
template void RayBeam::extractRays<std::list<Ray>>(
  std::list<Ray> &,
  uint32_t mask,
  OpticalSurface *);

template void RayBeam::extractRays<std::list<Ray>>(
  std::list<Ray> &,
  RayBeamSlice const &,
  uint32_t mask,
  OpticalSurface *);

template void RayBeam::extractRays<std::vector<Ray>>(
  std::vector<Ray> &,
  uint32_t mask,
  OpticalSurface *);

template void RayBeam::extractRays<std::vector<Ray>>(
  std::vector<Ray> &,
  RayBeamSlice const &,
  uint32_t mask,
  OpticalSurface *);

