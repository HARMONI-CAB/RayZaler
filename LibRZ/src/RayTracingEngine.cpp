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

#include <RayTracingEngine.h>
#include <ReferenceFrame.h>
#include <cstdlib>
#include <cstring>
#include <GenericAperture.h>
#include <Logger.h>
#include <exception>
#include <sys/param.h>
#include <OpticalElement.h>

using namespace RZ;


template<typename T>
static T *
allocBuffer(uint64_t count, T *existing = nullptr)
{
  if (count == 0)
    return nullptr;
  
  T *result = static_cast<T *>(realloc(existing, count * sizeof(T)));

  if (result == nullptr)
    throw std::bad_alloc();
  
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

template static Real *allocBuffer<Real>(uint64_t, Real *);
template static void  freeBuffer<Real>(Real *&);

template static uint64_t *allocBuffer<uint64_t>(uint64_t, uint64_t *);
template static void freeBuffer<uint64_t>(uint64_t *&);

void
RayBeam::clearMask()
{
  memset(mask, 0, ((count + 63) >> 6) << 3);
  memset(prevMask, 0, ((count + 63) >> 6) << 3);
}

void
RayBeam::clearStatistics()
{
  this->pruned      = 0;
  this->intercepted = 0;
}

void
RayBeam::interceptDone(uint64_t index)
{
  if (hitSaveSurface != nullptr) {
    Ray ray;
    if (extractPartialRay(ray, index, false)) {
      hitSaveSurface->hits.push_back(ray);
      ++this->intercepted;
    }
  }
}

void
RayBeam::allocate(uint64_t count)
{
  size_t oldMaskLen = (this->count + 63) >> 6;
  size_t maskLen = (count + 63) >> 6;

  if (this->count == 0) {
    this->origins       = allocBuffer<Real>(3 * count);
    this->directions    = allocBuffer<Real>(3 * count);
    this->normals       = allocBuffer<Real>(3 * count);
    this->destinations  = allocBuffer<Real>(3 * count);
    this->amplitude     = allocBuffer<Complex>(count);
    this->lengths       = allocBuffer<Real>(count);
    this->cumOptLengths = allocBuffer<Real>(count);
    this->ids           = allocBuffer<uint32_t>(count);
    this->mask          = allocBuffer<uint64_t>(maskLen);
    this->prevMask      = allocBuffer<uint64_t>(maskLen);
    this->chiefMask     = allocBuffer<uint64_t>(maskLen);
    this->allocation    = count;
  } else if (count >= this->count) {
    this->origins       = allocBuffer<Real>(3 * count, this->origins);
    this->directions    = allocBuffer<Real>(3 * count, this->directions);
    this->normals       = allocBuffer<Real>(3 * count, this->normals);
    this->destinations  = allocBuffer<Real>(3 * count, this->destinations);
    this->amplitude     = allocBuffer<Complex>(count, this->amplitude);
    this->lengths       = allocBuffer<Real>(count, this->lengths);
    this->cumOptLengths = allocBuffer<Real>(count, this->cumOptLengths);
    this->ids           = allocBuffer<uint32_t>(count, this->ids);
    this->mask          = allocBuffer<uint64_t>(maskLen, this->mask);
    this->prevMask      = allocBuffer<uint64_t>(maskLen, this->prevMask);
    this->chiefMask     = allocBuffer<uint64_t>(maskLen, this->chiefMask);
    this->allocation    = count;
  } else {
    throw std::runtime_error("Cannot shrink ray list");
  }

  memset(this->chiefMask + oldMaskLen, 0, (maskLen - oldMaskLen) * sizeof(uint64_t));
  this->count = count;
}

void
RayBeam::deallocate()
{
  freeBuffer(origins);
  freeBuffer(directions);
  freeBuffer(destinations);
  freeBuffer(normals);
  freeBuffer(lengths);
  freeBuffer(cumOptLengths);
  freeBuffer(amplitude);
  freeBuffer(ids);
  freeBuffer(mask);
  freeBuffer(prevMask);
  freeBuffer(chiefMask);
}

RayBeam::RayBeam(uint64_t count)
{
  allocate(count);
}

RayBeam::~RayBeam()
{
  deallocate();
}

/////////////////////////////// RayTransferProcessor ///////////////////////////
RayTransferProcessor::~RayTransferProcessor()
{
  if (m_aperture != nullptr)
    delete m_aperture;
}

//////////////////////////// RayTracingProcessListener /////////////////////////
void
RayTracingProcessListener::stageProgress(
  RayTracingStageProgressType,
  std::string const &,
  unsigned int num,
  unsigned int total)
{
  // NO-OP
}

void
RayTracingProcessListener::rayProgress(uint64_t num, uint64_t total)
{
  // NO-OP
}

uint64_t
RayTracingProcessListener::rayNotifyInterval() const
{
  return 250;
}

bool
RayTracingProcessListener::cancelled() const
{
  return false;
}

//////////////////////////////// RayTracingEngine //////////////////////////////
RayTracingEngine::RayTracingEngine()
{

}

RayTracingEngine::~RayTracingEngine()
{
  if (m_beam != nullptr)
    delete m_beam;
}

void
RayTracingEngine::clear()
{
  m_rays.clear();
  m_beamDirty = true;
}

RayTracingProcessListener *
RayTracingEngine::listener() const
{
  return m_listener;
}

void
RayTracingEngine::setListener(RayTracingProcessListener *listener)
{
  m_listener = listener;
}

void
RayTracingEngine::pushRay(
  Point3 const &origin,
  Vec3 const &direction,
  Real l,
  uint32_t id)
{
  Ray ray;

  ray.origin    = origin;
  ray.direction = direction.normalized();
  ray.length    = l;
  ray.id        = id;

  m_rays.push_back(ray);

  m_beamDirty = true;
}

void
RayTracingEngine::pushRays(std::list<Ray> const &rays)
{
  m_rays.insert(m_rays.end(), rays.begin(), rays.end());
  toBeam();

  // Assume rays come from a flat surface
  memcpy(m_beam->normals, m_beam->directions, 3 * m_beam->count * sizeof(Real));

  for (auto i = 0; i < m_beam->count; ++i)
    m_beam->amplitude[i] = 1;
}

void
RayTracingEngine::toBeam()
{
  uint64_t i = 0;

  if (m_beam == nullptr)
    m_beam = this->makeBeam();
  else
    m_beam->allocate(m_rays.size());

  m_beam->clearMask();

  for (auto p = m_rays.begin(); p != m_rays.end(); ++p) {
    p->origin.copyToArray(m_beam->origins + 3 * i);
    p->direction.copyToArray(m_beam->directions + 3 * i);

    m_beam->lengths[i]       = p->length;
    m_beam->cumOptLengths[i] = p->cumOptLength;
    m_beam->ids[i]           = p->id;
    
    if (p->chief)
      m_beam->setChiefRay(i);
    
    ++i;
  }

  m_beamDirty = false;
}

void
RayTracingEngine::toRays(bool keepPruned)
{
  uint64_t i;
  Ray ray;

  m_rays.clear();

  for (i = 0; i < m_beam->count; ++i) {
    if (m_beam->extractRay(ray, i, keepPruned))
      m_rays.push_back(ray);
  }

  m_raysDirty = false;
}

// Set the current surface. This also notifies the state.
void
RayTracingEngine::setCurrentSurface(
  const OpticalSurface *surf,
  unsigned int i,
  unsigned int total)
{
  m_currentSurface = surf;
  m_currentFrame   = surf == nullptr ? nullptr : surf->frame;

  m_currStage      = i;
  m_numStages      = total;

  m_beam->clearStatistics();
  m_beam->hitSaveSurface = 
    (surf->parent != nullptr && surf->parent->recordHits())
    ? surf
    : nullptr;

  if (surf != nullptr) {
    surf->clearCache();
    if (total > 0)
      stageProgress(PROGRESS_TYPE_TRACE, surf->name, i, total);
  }
}

void
RayTracingEngine::trace()
{
  if (m_beam == nullptr)
    m_beam = new RayBeam(m_rays.size());

  if (m_beamDirty)
    toBeam();

  cast(
    m_currentFrame->getCenter(),
    m_currentFrame->eZ(),
    m_currentSurface->processor->reversible());

  m_raysDirty = true;
  m_notificationPendig = false;
}

void
RayTracingEngine::propagatePhase()
{
  auto count = m_beam->count;
  Real K = m_K;

  for (auto i = 0; i < count; ++i)
    if (m_beam->hasRay(i))
      m_beam->amplitude[i] *= std::exp(Complex(0, K * m_beam->cumOptLengths[i]));
}

void
RayTracingEngine::updateNormals()
{
  memcpy(m_beam->normals, m_currNormals.data(), 3 * m_beam->count * sizeof(Real));
  m_dA = m_currdA;
}

void
RayTracingEngine::updateOrigins()
{
  memcpy(m_beam->origins, m_beam->destinations, 3 * m_beam->count * sizeof(Real));
  memcpy(m_beam->prevMask, m_beam->mask, ((m_beam->count + 63) >> 6) << 3);
}

void
RayTracingEngine::transfer()
{
  if (m_currentSurface == nullptr)
    throw std::runtime_error("Cannot transfer: optical surface not defined");
  
  stageProgress(
    PROGRESS_TYPE_TRANSFER,
    m_currentSurface->name,
    m_currStage,
    m_numStages);
  
  auto aperture = m_currentSurface->processor->aperture();
  auto count = m_beam->count;

  m_currentSurface->processor->process(*m_beam, m_currentFrame);

  if (m_beam->hitSaveSurface != nullptr) {
    auto surf = const_cast<OpticalSurface *>(m_beam->hitSaveSurface);
    surf->intercepted += m_beam->intercepted;
    surf->pruned      += m_beam->pruned;
  }

  propagatePhase();

  m_raysDirty = true;
}

std::list<Ray> const &
RayTracingEngine::getRays(bool keepPruned)
{
  if (m_raysDirty)
    toRays(keepPruned);

  return m_rays;
}

RayBeam *
RayTracingEngine::makeBeam()
{
  return new RayBeam(m_rays.size());
}

void
RayTracingEngine::tick()
{
  gettimeofday(&m_start, nullptr);
}

void
RayTracingEngine::setStartTime(struct timeval const &tv)
{
  m_start = tv;
}

uint64_t
RayTracingEngine::tack() const
{
  struct timeval tv, diff;

  gettimeofday(&tv, nullptr);
  timersub(&tv, &m_start, &diff);

  return static_cast<uint64_t>(diff.tv_sec) * 1000ull
         + static_cast<uint64_t>(diff.tv_usec / 1000);
}

struct timeval
RayTracingEngine::lastTick() const
{
  return m_start;
}

bool
RayTracingEngine::notificationPending() const
{
  return m_notificationPendig;
}

void
RayTracingEngine::clearPendingNotifications()
{
  m_notificationPendig = false;
}

bool
RayTracingEngine::cancelled() const
{
  if (m_listener != nullptr)
    return m_listener->cancelled();

  return false;
}

void
RayTracingEngine::rayProgress(uint64_t num, uint64_t total)
{
  if (m_listener != nullptr) {
    if (tack() > m_listener->rayNotifyInterval()) {
      m_listener->rayProgress(num, total);
      m_notificationPendig = true;
      tick();
    }
  }
}

void
RayTracingEngine::stageProgress(
  RayTracingStageProgressType type,
  std::string const &text,
  unsigned int num,
  unsigned int total)
{
  if (m_listener != nullptr) {
    if (m_notificationPendig || tack() > m_listener->rayNotifyInterval()) {
      m_listener->stageProgress(type, text, num, total);
      if (m_beam != nullptr) {
        if (type == PROGRESS_TYPE_TRANSFER)
          rayProgress(m_beam->count * 3, m_beam->count * 3);
        else if (type == PROGRESS_TYPE_TRACE)
          rayProgress(0, m_beam->count * 3);
      }

      if (!m_notificationPendig)
        m_notificationPendig = true;
    }
  }
}
