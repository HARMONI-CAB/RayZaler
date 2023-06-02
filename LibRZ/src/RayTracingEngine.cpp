#include <RayTracingEngine.h>
#include <ReferenceFrame.h>
#include <cstdlib>
#include <cstring>

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
}

void
RayBeam::allocate(uint64_t count)
{
  if (this->count == 0) {
    this->origins      = allocBuffer<Real>(3 * count);
    this->directions   = allocBuffer<Real>(3 * count);
    this->destinations = allocBuffer<Real>(3 * count);
    this->lengths      = allocBuffer<Real>(count);
    this->mask         = allocBuffer<uint64_t>((count + 63) >> 6);
    this->allocation   = count;
  } else if (count >= this->count) {
    this->origins      = allocBuffer<Real>(3 * count, this->origins);
    this->directions   = allocBuffer<Real>(3 * count, this->directions);
    this->destinations = allocBuffer<Real>(3 * count, this->destinations);
    this->lengths      = allocBuffer<Real>(count, this->lengths);
    this->mask         = allocBuffer<uint64_t>((count + 63) >> 6, this->mask);
    this->allocation   = count;
  }

  this->count = count;
}

void
RayBeam::deallocate()
{
  freeBuffer(origins);
  freeBuffer(directions);
  freeBuffer(destinations);
  freeBuffer(lengths);
  freeBuffer(mask);
}

RayBeam::RayBeam(uint64_t count)
{
  allocate(count);
}

RayBeam::~RayBeam()
{
  deallocate();
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

void
RayTracingEngine::pushRay(Point3 const &origin, Vec3 const &direction, Real l)
{
  Ray ray;

  ray.origin    = origin;
  ray.direction = direction.normalized();
  ray.length    = l;

  m_rays.push_back(ray);

  m_beamDirty = true;
}

void
RayTracingEngine::pushRays(std::list<Ray> const &rays)
{
  m_rays.insert(m_rays.end(), rays.begin(), rays.end());
  m_beamDirty = true;
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
    m_beam->origins[3 * i + 0] = p->origin.x;
    m_beam->origins[3 * i + 1] = p->origin.y;
    m_beam->origins[3 * i + 2] = p->origin.z;

    m_beam->directions[3 * i + 0] = p->direction.x;
    m_beam->directions[3 * i + 1] = p->direction.y;
    m_beam->directions[3 * i + 2] = p->direction.z;

    m_beam->lengths[i] = p->length;

    ++i;
  }

  m_beamDirty = false;
}

void
RayTracingEngine::toRays()
{
  uint64_t i;
  Ray ray;

  m_rays.clear();

  for (i = 0; i < m_beam->count; ++i) {
    if (m_beam->extractRay(ray, i))
      m_rays.push_back(ray);
  }

  m_raysDirty = false;
}

void
RayTracingEngine::trace(const ReferenceFrame *surface)
{
  if (m_beam == nullptr)
    m_beam = new RayBeam(m_rays.size());

  if (m_beamDirty)
    toBeam();

  m_current = surface;
  cast(surface->getCenter(), surface->eZ());

  m_raysDirty = true;
}

void
RayTracingEngine::transfer(const RayTransferProcessor *processor)
{
  processor->process(*m_beam, m_current);

  m_raysDirty = true;
}

std::list<Ray> const &
RayTracingEngine::getRays()
{
  if (m_raysDirty)
    toRays();

  return m_rays;
}

RayBeam *
RayTracingEngine::makeBeam()
{
  return new RayBeam(m_rays.size());
}
