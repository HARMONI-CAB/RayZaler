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
#include <SurfaceShape.h>
#include <Logger.h>
#include <exception>
#include <sys/param.h>
#include <OpticalElement.h>

using namespace RZ;

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

  if (m_beam != nullptr) {
    delete m_beam;
    m_beam = nullptr;
  }
  
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
RayTracingEngine::pushRays(RayList const &rays)
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
    if (p->wavelength <= RZ_BEAM_MINIMUM_WAVELENGTH)
      throw std::runtime_error(
        string_printf(
          "Wavelength is too short (minimum: %g pm)",
          RZ_BEAM_MINIMUM_WAVELENGTH * 1e12));
          
    p->origin.copyToArray(m_beam->origins + 3 * i);
    p->origin.copyToArray(m_beam->destinations + 3 * i);
    p->direction.copyToArray(m_beam->directions + 3 * i);

    m_beam->lengths[i]       = p->length;
    m_beam->cumOptLengths[i] = p->cumOptLength;
    m_beam->ids[i]           = p->id;
    m_beam->wavelengths[i]   = p->wavelength;
    m_beam->refNdx[i]        = p->refNdx;

    if (p->chief)
      m_beam->setChiefRay(i);
    
    ++i;
  }

  m_beamDirty = false;
}

void
RayTracingEngine::toRays(bool keepPruned)
{
  m_rays.clear();

  m_beam->extractRays(m_rays, OriginPOV | ExtractAll);

  m_raysDirty = false;
}

void
RayTracingEngine::castTo(const OpticalSurface *surface, RayBeam *beam)
{
  if (beam == nullptr) {
    beam = ensureMainBeam();
    beam->toRelative(surface->frame);
    m_raysDirty = true;
  }

  // Update progress
  stageProgress(PROGRESS_TYPE_TRACE, m_stageName, m_currStage, m_numStages);

  beam->uninterceptAll();

  cast(surface, beam);

  m_notificationPendig = false;
}

void
RayTracingEngine::transmitThrough(const OpticalSurface *surface)
{
  assert(m_beam != nullptr);
  assert(m_beam->nonSeq == (surface == nullptr));
  
  stageProgress(PROGRESS_TYPE_TRANSFER, m_stageName, m_currStage, m_numStages);

  transmit(surface, m_beam);

  if (surface != nullptr)
    m_beam->fromRelative(surface->frame);
  else
    m_beam->fromSurfaceRelative();

  m_raysDirty = true;
}

void
RayTracingEngine::transmitThroughIntercepted()
{
  transmitThrough(nullptr);
}

void
RayTracingEngine::updateOrigins()
{
  m_beam->updateOrigins();
}

RayList const &
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

RayBeam *
RayTracingEngine::makeNSBeam()
{
  auto nsBeam = new RayBeam(beam()->count, true);

  beam()->copyTo(nsBeam);

  return nsBeam;
}

RayBeam *
RayTracingEngine::ensureMainBeam()
{
  if (m_beam == nullptr || m_beamDirty)
    toBeam();

  return m_beam;
}

void
RayTracingEngine::setMainBeam(RayBeam *original)
{
  if (m_beam != nullptr)
    delete m_beam;
  
  m_beam = original;

  m_raysDirty = true;
  m_beamDirty = false;
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
