#include <RayTracingEngine.h>
#include <ReferenceFrame.h>
#include <cstdlib>
#include <cstring>
#include <GenericAperture.h>
#include <Logger.h>
#include <exception>
#include <sys/param.h>

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
    this->origins       = allocBuffer<Real>(3 * count);
    this->directions    = allocBuffer<Real>(3 * count);
    this->normals       = allocBuffer<Real>(3 * count);
    this->destinations  = allocBuffer<Real>(3 * count);
    this->amplitude     = allocBuffer<Complex>(count);
    this->lengths       = allocBuffer<Real>(count);
    this->cumOptLengths = allocBuffer<Real>(count);
    this->mask          = allocBuffer<uint64_t>((count + 63) >> 6);
    this->allocation    = count;
  } else if (count >= this->count) {
    this->origins       = allocBuffer<Real>(3 * count, this->origins);
    this->directions    = allocBuffer<Real>(3 * count, this->directions);
    this->normals       = allocBuffer<Real>(3 * count, this->normals);
    this->destinations  = allocBuffer<Real>(3 * count, this->destinations);
    this->amplitude     = allocBuffer<Complex>(count, this->amplitude);
    this->lengths       = allocBuffer<Real>(count, this->lengths);
    this->cumOptLengths = allocBuffer<Real>(count, this->cumOptLengths);
    this->mask          = allocBuffer<uint64_t>((count + 63) >> 6, this->mask);
    this->allocation    = count;
  }

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
  m_notificationPendig = false;
}

//
// TODO: Compute random destinations, calculate t and update opticalLengths
// 

void
RayTracingEngine::updateDirections()
{
  unsigned int i;

  if (m_beam == nullptr)
    return;

  auto count = m_beam->count;

  for (unsigned int i = 0; i < count; ++i) {
    Vec3 destination(m_beam->destinations + 3 * i);
    Vec3 origin(m_beam->origins + 3 * i);
    Vec3 diff = destination - origin;
    Real dt = diff.norm();

    (diff * (1/dt)).copyToArray(m_beam->directions + 3 * i);

    m_beam->lengths[i] = dt;
    m_beam->cumOptLengths[i] += dt; // TODO: Multiply by refractive index
  }
}

//
// This is the core of our diffraction calculations. This assumes the following:
//
// - Wave vector of the rays that hitted the depart surface are encoded in
//   m_beam->directions
// - Wave vector of the rays hitting the arrival surface are encoded in
//   the difference between destination and origin
// - Surface normals of the depart surface are known, and saved in the beam.
//   After kirchoff, currNormals should be transferred back to the beam.
//

//
// D = 1.2, flen = 0.6
// dTheta = 1.22 * lambda / D
// dX     = f * dTheta = 1.22 * lambda * f/#
// pxWidth = 2e-3
// dx = 5 * pxWidth
// 5 * pxWidth = 1.22 * lambda * f/#
// lambda = 5 * pxWidth / (1.22 * f/#)
//

void
RayTracingEngine::integrateKirchhoff()
{
  auto count = m_beam->count;
  Real K = 2 * M_PI * 1.22 * (0.6 / 1.2) / (5 * 2e-3);

  for (auto i = 0; i < count; ++i) {
    Vec3 normal      = Vec3(m_beam->normals + 3 * i);
    Vec3 kivec       = K * Vec3(m_beam->directions + 3 * i);
    Vec3 destination = Vec3(m_beam->destinations + 3 * i);
    Vec3 origin      = Vec3(m_beam->origins + 3 * i);
    Vec3 rvec        = destination - origin;
    Real r           = rvec.norm();
    Vec3 d           = rvec / r;
    Vec3 kvec        = K * d;

    if (normal * kvec < 0)
      normal = -normal;
    
    Real QAmpl       = (kvec - kivec) * normal;
    Real IAmpl       = (-d / r) * normal;
    Real phi         = fmod(K * r, 2 * M_PI);

    Complex factor = Complex(IAmpl, QAmpl) * std::exp(Complex(0, phi)) / r;

    m_beam->amplitude[i] *= factor * m_dA;

    rayProgress(i, count);
  }
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
}

void
RayTracingEngine::traceFromInfinity(
  const ReferenceFrame *surface,
  const RayTransferProcessor *processor)
{
  auto aperture = processor->aperture();

  if (m_beam == nullptr)
    m_beam = new RayBeam(m_rays.size());

  if (m_beamDirty)
    toBeam();
  
  // Stage 1: Trace rays
  m_current = surface;
  cast(surface->getCenter(), surface->eZ());

  // Stage 2: Intercept
  auto count = m_beam->count;
  m_currNormals.resize(3 * count);
  m_currdA = aperture->area() / count;

  for (auto i = 0; i < count; ++i) {
    if (!m_beam->hasRay(i))
      continue;

    Vec3 coord  = surface->toRelative(Vec3(m_beam->destinations + 3 * i));
    Vec3 origin = surface->toRelative(Vec3(m_beam->origins + 3 * i));
    Vec3 normal;
    Real dt;

    if (aperture->intercept(coord, normal, dt, origin)) {
      m_beam->lengths[i]       += dt;
      m_beam->cumOptLengths[i] += m_beam->n * dt;

      // Stage 3: Update beam
      surface->fromRelative(coord).copyToArray(m_beam->destinations + 3 * i);
      surface->fromRelativeVec(normal).copyToArray(m_currNormals.data() + 3 * i);
    } else {
      m_beam->prune(i);
    }

    rayProgress(i, count);
  }

  m_raysDirty = true;
  m_notificationPendig = false;
}

void
RayTracingEngine::traceRandom(
  const ReferenceFrame *surface,
  const RayTransferProcessor *processor)
{
  auto *aperture = processor->aperture();

  if (m_beam == nullptr)
    m_beam = new RayBeam(m_rays.size());

  auto count = m_beam->count;
  
  m_currNormals.resize(3 * count);
  m_currdA = aperture->area() / count;

  if (m_beamDirty)
    toBeam();

  m_current = surface;
  if (aperture == nullptr) {
    RZError(
      "Processor %s with undefined aperture found\n",
      processor->name().c_str());
    return;
  }

  aperture->generatePoints(
    m_current,
    m_beam->destinations,
    m_currNormals.data(),
    count);

  m_raysDirty = true;
  m_notificationPendig = false;
}
    

void
RayTracingEngine::transfer(const RayTransferProcessor *processor)
{
  auto aperture = processor->aperture();
  auto count = m_beam->count;

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
