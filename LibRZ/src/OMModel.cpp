#include <OMModel.h>
#include <Element.h>
#include <RayBeamElement.h>
#include <CPURayTracingEngine.h>
#include <RotatedFrame.h>
#include <TranslatedFrame.h>
#include <Logger.h>
#include <sys/time.h>
#include <GenericAperture.h>

#define TRACE_PROGRESS_INTERVAL_MS 250

using namespace RZ;

//
// I will allow stuff like:
// frames.<framename>.<param>
// elements.<elementname>.<param>
// dofs.<param>
// params.<param>
//


/*
Having  particular  symbols  (variable  or  function)  present  in  an
expression is one form of dependency. Another and just as  interesting
and important type of  dependency is that of  assignments. Assignments
are the set of dependent symbols that 'may' have their values modified
within an expression. The following are example expressions and  their
associated assignments:

       Assignments   Expression
   (1) x             x := y + z
   (2) x, y          x += y += z
   (3) x, y, z       x := y += sin(z := w + 2)
   (4) w, z          if (x > y, z := x + 2, w := 'A String')
   (5) None          x + y + z


Note: In expression 4, both variables 'w' and 'z' are denoted as being
assignments even though only one of  them can ever be modified at  the
time of evaluation. Furthermore the determination of which of the  two
variables the  modification will  occur upon  can only  be known  with
certainty at evaluation time and not beforehand, hence both are listed
as being candidates for assignment.

The following builds upon the previous example demonstrating the usage
of the DEC in determining the 'assignments' of the given expression:

   //Collect assignments
   parser.dec().collect_assignments() = true;

   if (!parser.compile(expression_string,expression))
   {
      // error....
   }

   std::deque<symbol_t> symbol_list;

   parser.dec().assignment_symbols(symbol_list);

   for (std::size_t i = 0; i < symbol_list.size(); ++i)
   {
      symbol_t& symbol = symbol_list[i];

      switch (symbol.second)
      {
         case parser_t::e_st_variable : ... break;
         case parser_t::e_st_vector   : ... break;
         case parser_t::e_st_string   : ... break;
      }
   }
   */

ReferenceFrameContext &
ReferenceFrameContext::rotate(Real angle, Vec3 const &axis)
{
  ReferenceFrame *rotatedFrame = new RotatedFrame(
    m_model->genReferenceFrameName("Rotation"),
    m_last,
    axis,
    angle);

  (void) m_model->registerFrame(rotatedFrame);

  m_last = rotatedFrame;
  
  return *this;
}

ReferenceFrameContext &
ReferenceFrameContext::rotate(Real angle, Real aX, Real aY, Real aZ)
{
  return rotate(angle, Vec3(aX, aY, aZ));
}

ReferenceFrameContext &
ReferenceFrameContext::translate(Vec3 const &disp)
{
  ReferenceFrame *translFrame = new TranslatedFrame(
    m_model->genReferenceFrameName("Translation"),
    m_last,
    disp);

  (void) m_model->registerFrame(translFrame);

  m_last = translFrame;
  
  return *this;
}

ReferenceFrameContext &
ReferenceFrameContext::translate(Real dx, Real dy, Real dz)
{
  return translate(Vec3(dx, dy, dz));
}

bool
OMModel::registerFrame(ReferenceFrame *frame)
{
  if (m_nameToFrame.find(frame->name()) != m_nameToFrame.end())
    return false;

  m_frames.push_back(frame);
  m_nameToFrame[frame->name()] = frame;

  return true;
}

void
OMModel::setFrameAlias(ReferenceFrame *frame, std::string const &alias)
{
  m_nameToFrame[alias] = frame;
}

bool
OMModel::registerElement(Element *element)
{
  if (m_nameToElement.find(element->name()) != m_nameToElement.end())
    return false;

  m_elements.push_back(element);
  m_nameToElement[element->name()] = element;

  // Register all names
  auto ports = element->ports();

  for (auto portName : ports) {
    std::string name = element->name() + "." + portName;
    m_nameToFrame[name] = element->getPortFrame(portName);
  }

  return true;
}

bool
OMModel::registerOpticalElement(OpticalElement *element)
{
  if (!registerElement(element))
    return false;

  m_nameToOpticalElement[element->name()] = element;

  return true;
}

bool
OMModel::registerDetector(Detector *element)
{
  if (!registerOpticalElement(element))
    return false;

  m_nameToDetector[element->name()] = element;

  return true;
}

bool
OMModel::registerDetectorAlias(std::string const &name, Detector *det)
{
  if (m_nameToDetector.find(name) == m_nameToDetector.end()) {
    m_nameToDetector[name] = det;
    return true;
  }

  return false;
}

Element *
OMModel::autoRegisterElement(Element *newElement)
{
  Element *retElement = nullptr;
  bool ret;
  auto fact = newElement->factory();

  if (fact != nullptr && fact->name() == "Detector")
    ret = registerDetector(static_cast<Detector *>(newElement));
  else if (newElement->hasProperty("optical"))
    ret = registerOpticalElement(static_cast<OpticalElement *>(newElement));
  else
    ret = registerElement(newElement);

  if (!ret)
    delete newElement;
  else
    retElement = newElement;

  return retElement;
}

// Lookup methods
ReferenceFrame *
OMModel::lookupReferenceFrame(std::string const &name) const
{
  auto map = &m_nameToFrame;
  auto p = map->find(name);

  if (p == map->end())
    return nullptr;

  return p->second;
}

Element *
OMModel::lookupElement(std::string const &name) const
{
  auto map = &m_nameToElement;
  auto p = map->find(name);

  if (p == map->end())
    return nullptr;

  return p->second;
}

OpticalElement *
OMModel::lookupOpticalElement(std::string const &name) const
{
  auto map = &m_nameToOpticalElement;
  auto p = map->find(name);

  if (p == map->end())
    return nullptr;

  return p->second;
}

Detector *
OMModel::lookupDetector(std::string const &name) const
{
  auto map = &m_nameToDetector;
  auto p = map->find(name);

  if (p == map->end())
    return nullptr;

  return p->second;
}

const OpticalPath *
OMModel::lookupOpticalPath(std::string const &name) const
{
  auto map = &m_nameToPath;
  auto p = map->find(name);

  if (p == map->end())
    return nullptr;

  return p->second;
}

// Lookup methods (raise exception)
ReferenceFrame *
OMModel::lookupReferenceFrameOrEx(std::string const &name) const
{
  auto ret = lookupReferenceFrame(name);

  if (ret == nullptr)
    throw std::runtime_error("Reference frame `" + name + "` does not exist");

  return ret;
}

Element *
OMModel::lookupElementOrEx(std::string const &name) const
{
  auto ret = lookupElement(name);

  if (ret == nullptr)
    throw std::runtime_error("Element `" + name + "` does not exist");

  return ret;
}

OpticalElement *
OMModel::lookupOpticalElementOrEx(std::string const &name) const
{
  auto ret = lookupOpticalElement(name);

  if (ret == nullptr)
    throw std::runtime_error("Optical element `" + name + "` does not exist");

  return ret;
}

Detector *
OMModel::lookupDetectorOrEx(std::string const &name) const
{
  auto ret = lookupDetector(name);

  if (ret == nullptr)
    throw std::runtime_error("Detector `" + name + "` does not exist");

  return ret;
}

const OpticalPath *
OMModel::lookupOpticalPathOrEx(std::string const &name) const
{
  auto ret = lookupOpticalPath(name);

  if (ret == nullptr) {
    if (name.size() == 0)
      throw std::runtime_error("Default optical path does not exist");
    else
      throw std::runtime_error("Optical path `" + name + "` does not exist");
  }

  return ret;
}

std::string
OMModel::genElementName(std::string const &type)
{
  std::string hint;
  unsigned int i = 1;

  if (lookupElement(type) == nullptr)
    return type;

  do {
    hint = type + "_" + std::to_string(i++);
  } while (lookupElement(hint) != nullptr);

  return hint;
}

std::string
OMModel::genReferenceFrameName(std::string const &type)
{
  std::string hint;
  unsigned int i = 1;

  if (lookupReferenceFrame(type) == nullptr)
    return type;

  do {
    hint = type + "_" + std::to_string(i++);
  } while (lookupReferenceFrame(hint) != nullptr);

  return hint;
}

bool
OMModel::addOpticalPath(std::string const &name, std::list<std::string> const &list)
{
  std::list<std::pair<OpticalElement *, std::string>> elements;

  if (lookupOpticalPath(name) != nullptr)
    return false;

  for (auto p : list) {
    auto dot = p.find('.');
    std::string pathName = "";

    if (dot != std::string::npos) {
      pathName = p.substr(dot + 1);
      p        = p.substr(0, dot);
    }

    elements.push_back(std::pair(lookupOpticalElementOrEx(p), pathName));
  }

  OpticalElement *prev = nullptr;
  OpticalPath *path = nullptr;

  for (auto curr : elements) {
    if (prev == nullptr) {
      m_paths.push_back(curr.first->opticalPath(curr.second));
      path = &m_paths.back();
    } else {
      path->plug(curr.first, curr.second);
    }

    prev = curr.first;
  }

  m_nameToPath[name] = path;
  
  return true;
}

void
OMModel::recalculate()
{
  world()->recalculate();
}

ReferenceFrameContext
OMModel::rotate(Real angle, Vec3 const &axis, ReferenceFrame *parent)
{
  ReferenceFrameContext ctx;

  if (parent == nullptr)
    parent = world();

  ReferenceFrame *rotatedFrame = new RotatedFrame(
    genReferenceFrameName("Rotation"),
    parent,
    axis,
    angle);

  (void) registerFrame(rotatedFrame);

  ctx.m_last = rotatedFrame;
  ctx.m_model = this;
  
  return ctx;
}

ReferenceFrameContext
OMModel::rotate(Real angle, Real aX, Real aY, Real aZ, ReferenceFrame *parent)
{
  return rotate(angle, Vec3(aX, aY, aZ), parent);
}

ReferenceFrameContext
OMModel::translate(Vec3 const &disp, ReferenceFrame *parent)
{
  ReferenceFrameContext ctx;

  if (parent == nullptr)
    parent = world();

  ReferenceFrame *translFrame = new TranslatedFrame(
    genReferenceFrameName("Translation"),
    parent,
    disp);

  (void) registerFrame(translFrame);

  ctx.m_last = translFrame;
  ctx.m_model = this;
  
  return ctx;
}

ReferenceFrameContext
OMModel::translate(Real dx, Real dy, Real dz, ReferenceFrame *parent)
{
  return translate(Vec3(dx, dy, dz), parent);
}

bool
OMModel::addDetector(
  std::string const &name,
  ReferenceFrame *frame,
  unsigned int cols,
  unsigned int rows,
  Real width,
  Real height)
{
  auto sing = Singleton::instance();
  auto factory = sing->lookupElementFactory("Detector");
  Detector *detector = 
    static_cast<Detector *>(factory->make(name, frame));

  detector->set("height", height);
  detector->set("width", width);
  detector->set("cols", cols);
  detector->set("rows", rows);

  return registerDetector(detector);
}

bool
OMModel::addDetector(
  std::string const &name,
  std::string const &parentFrame,
  unsigned int cols,
  unsigned int rows,
  Real width,
  Real height)
{
  if (lookupDetector(name) != nullptr)
    return false;

  return addDetector(
    name,
    lookupReferenceFrameOrEx(parentFrame),
    cols,
    rows,
    width,
    height);
}

// Enumeration methods
std::list<std::string>
OMModel::frames() const
{
  std::list<std::string> keys;
  for (auto p : m_nameToFrame)
    keys.push_back(p.first);

  return keys;
}

std::list<std::string>
OMModel::elements() const
{
  std::list<std::string> keys;
  for (auto p : m_nameToElement)
    keys.push_back(p.first);

  return keys;
}

std::list<std::string>
OMModel::opticalElements() const
{
  std::list<std::string> keys;
  for (auto p : m_nameToOpticalElement)
    keys.push_back(p.first);

  return keys;
}

std::list<std::string>
OMModel::detectors() const
{
  std::list<std::string> keys;
  for (auto p : m_nameToDetector)
    keys.push_back(p.first);

  return keys;
}

std::list<std::string>
OMModel::opticalPaths() const
{
  std::list<std::string> keys;
  for (auto p : m_nameToPath)
    keys.push_back(p.first);

  return keys;
}

// Convenience methods (fast enumeration)
std::list<Element *> const &
OMModel::elementList() const
{
  return m_elements;
}

Element *
OMModel::beam() const
{
  return static_cast<Element *>(m_beam);
}

void
OMModel::clearBeam()
{
  m_beam->setList(std::list<RZ::Ray>());
}

//
// The tracing process is a loop that works as folows:
// 
// 1. Input rays are pushed to the tracer
// 2. An element is extracted from the optical surface list. The rays
//    have a defined origin and direction.
// 3. The rays are intersected with the next optical surface with trace()
// 4. The true intersection points and next origin / direction are placed back
//    in the current beam.
// 5. We repeat from 2.
//

bool
OMModel::traceDiffraction(
        std::string const &pathName,
        std::list<Ray> const &rays,
        RayTracingProcessListener *listener,
        bool clear,
        const struct timeval *startTime)
{
  CPURayTracingEngine tracer;
  struct timeval tv;
  unsigned int n, stages;
  const OpticalPath *path = lookupOpticalPathOrEx(pathName);
  bool prev = false;

  m_intermediateRays.clear();

  tracer.setK(2 * M_PI / m_wavelength);
  tracer.setListener(listener);

  gettimeofday(&tv, nullptr);

  m_randState.setSeed(tv.tv_sec + tv.tv_usec / 1000);

  // Clear all detectors
  if (clear)
    for (auto p : m_nameToDetector)
      p.second->clear();

  tracer.pushRays(rays);

  stages = path->m_sequence.size();
  n      = 0;

  if (startTime != nullptr)
    tracer.setStartTime(*startTime);
  else
    tracer.tick();

  for (auto p : path->m_sequence) {
    ++n;

    tracer.stageProgress(PROGRESS_TYPE_TRACE, p.name, n, stages);

    //tracer.trace(p.frame);

    // Trace random
    p.processor->aperture()->randState() = m_randState;

    if (prev)
      tracer.traceRandom(p.frame, p.processor);
    else
      tracer.traceFromInfinity(p.frame, p.processor);

    m_randState = p.processor->aperture()->randState();
    if (listener != nullptr && listener->cancelled())
      return false;

    // Okay, after this step we have the following situation:
    // - Origin is defined
    // - Destination is defined
    // - Direction encodes the direction at which the ray arrived to the depart surface
    // - Difference D - O encodes the new direction
    // - Normal of the previous surface is in normals
    // Additionally, the tracer keeps the normals of the new surface 
    
    tracer.stageProgress(PROGRESS_TYPE_KIRCHHOFF, p.name, n, stages);
    tracer.integrateKirchhoff();

    // - Amplitudes are up-to date now
    if (p.processor->name() == "DetectorProcessor") {
      tracer.stageProgress(PROGRESS_TYPE_TRANSFER, p.name, n, stages);
      tracer.transfer(p.processor);
    }

    //
    // After the following step, the direction encodes the unitary vector
    // pointing from the departure surface to the destination surface. This
    // is, the D - O normalized vector
    //
    tracer.updateDirections();
    
    //
    // After the following step, the normals generated by the aperture are
    // turned into normals of the departure surface
    //
    tracer.updateNormals();
    
    //
    // And finally, the destination points are the new origin points
    //
    tracer.updateOrigins();

    if (tracer.cancelled())
      return false;

    prev = true;
  }

  m_lastTick = tracer.lastTick();

  return true;
}

void
OMModel::setWavelength(Real wl)
{
  m_wavelength = wl;
}

Real
OMModel::wavelength() const
{
  return m_wavelength;
}

void
OMModel::setFrequency(Real freq)
{
  setWavelength(RZ_SPEED_OF_LIGHT / freq);
}

Real
OMModel::frequency() const
{
  return RZ_SPEED_OF_LIGHT / m_wavelength;
}

bool
OMModel::trace(
        std::string const &pathName,
        std::list<Ray> const &rays,
        bool updateBeamElement,
        RayTracingProcessListener *listener,
        bool clear,
        const struct timeval *startTime,
        bool clearIntermediate)
{
  CPURayTracingEngine tracer;
  struct timeval tv;
  unsigned int n, stages;
  const OpticalPath *path = lookupOpticalPathOrEx(pathName);
  bool prev = false;

  if (clearIntermediate)
    m_intermediateRays.clear();

  tracer.setK(2 * M_PI / m_wavelength);
  tracer.setListener(listener);

  gettimeofday(&tv, nullptr);

  m_randState.setSeed(tv.tv_sec + tv.tv_usec / 1000);

  // Clear all detectors
  if (clear)
    for (auto p : m_nameToDetector)
      p.second->clear();

  tracer.pushRays(rays);

  stages = path->m_sequence.size();
  n      = 0;

  if (startTime != nullptr)
    tracer.setStartTime(*startTime);
  else
    tracer.tick();

  for (auto p : path->m_sequence) {
    ++n;

    tracer.stageProgress(PROGRESS_TYPE_TRACE, p.name, n, stages);
    tracer.trace(p.frame);

    // Check if tracer was cancelled at this point
    if (listener != nullptr && listener->cancelled())
      return false;

    // - Amplitudes are up-to date now
    tracer.stageProgress(PROGRESS_TYPE_TRANSFER, p.name, n, stages);
    tracer.transfer(p.processor);

    if (updateBeamElement) {
      auto rays = tracer.getRays(true);

      m_intermediateRays.insert(
        m_intermediateRays.end(),
        rays.begin(),
        rays.end());
    }
    
    //
    // And finally, the destination points are the new origin points
    //
    tracer.updateOrigins();

    if (tracer.cancelled())
      return false;

    prev = true;
  }

  // TODO: make beam thread-safe
  if (updateBeamElement)
    m_beam->setList(m_intermediateRays);

  m_lastTick = tracer.lastTick();

  return true;
}

bool
OMModel::traceDefault(
    std::list<Ray> const &rays,
    bool updateBeamElement,
    RayTracingProcessListener *listener,
    bool clear,
    const struct timeval *startTime)
{
  return trace("", rays, updateBeamElement, listener, clear, startTime);
}

struct timeval
OMModel::lastTracerTick() const
{
  return m_lastTick;
}

bool
OMModel::savePNG(
  std::string const &detector,
  std::string const &file)
{
  lookupDetectorOrEx(detector)->savePNG(file);

  return false;
}

void
OMModel::setBeamColoring(RayColoring const *coloring)
{
  m_beam->setRayColoring(coloring);
}

struct UniformCircleSampler {
  Real radius, R2;
  Real dh;
  Real x0, y0, y;
  unsigned int j;

  inline void setSampling(Real R, unsigned N);
  inline bool sample(Vec3 &dest, Matrix3 const &sys, Vec3 const &center);
};

void
UniformCircleSampler::setSampling(Real R, unsigned N)
{
  R2      = R * R;
  radius  = R;
  Real dA = (M_PI * R2) / N;
  dh      = sqrt(dA);
  x0      = -dh * floor(2 * radius / dh) / 2;
  y0      = -dh * (floor(sqrt((R2 - x0 * x0)) / dh * 2) / 2);
  j = 0;
}

bool
UniformCircleSampler::sample(Vec3 &dest, Matrix3 const &sys, Vec3 const &center)
{
  y = y0 + j++ * dh;

  if (y > fabs(y0)) {
    x0 += dh;
    if (x0 > radius)
      return false;
    
    y0  = -dh * (floor(sqrt((R2 - x0 * x0)) / dh - .5) + .5);
    y   = y0;
    j   = 0;
  }

  dest = x0 * sys.row.vx + y * sys.row.vy + center;

  return true;
}

void
OMModel::addSkyBeam(
  std::list<Ray> &dest,
  unsigned int number,
  Real radius,
  Real azimuth,
  Real elevation,
  Real distance,
  uint32_t id,
  bool random)
{
  Matrix3 starSys   = Matrix3::azel(deg2rad(azimuth), deg2rad(elevation));
  Vec3 sourceCenter = distance * starSys.row.vz;
  Vec3 direction    = -starSys.row.vz;
  Ray  ray;
  UniformCircleSampler uSamp;
  Vec3 source;

  if (!random)
    uSamp.setSampling(radius, number);
  
  for (auto i = 0; i < number || !random; ++i) {
    if (random) {
      Real sep      = radius * sqrt(.5 * (1 + RZ_URANDSIGN));
      Real angle    = RZ_URANDSIGN * M_PI;
      source        = sep * (cos(angle) * starSys.row.vx + sin(angle) * starSys.row.vy) + sourceCenter;
    } else {
      if (!uSamp.sample(source, starSys, sourceCenter))
        break;
    }

    ray.origin    = source;
    ray.direction = direction;
    ray.length    = distance;
    ray.id        = id;
    
    dest.push_back(ray);
  }
}
void
OMModel::addElementRelativeBeam(
  std::list<Ray> &dest,
  Element *element,
  unsigned int number,
  Real radius,
  Real azimuth,
  Real elevation,
  Real offX,
  Real offY,
  Real distance,
  uint32_t id,
  bool random)
{
  const ReferenceFrame *frame;

  if (element->hasProperty("optical")) {
    OpticalElement *optEl = static_cast<OpticalElement *>(element);
    frame = optEl->opticalPath().m_sequence.begin()->frame;
  } else {
     frame = element->parentFrame();
  }
  Matrix3 elOrient  = frame->getOrientation();
  Vec3    elCenter  = frame->getCenter();
  Matrix3 beamSys   = Matrix3::azel(deg2rad(azimuth), deg2rad(elevation));
  Matrix3 orient    = beamSys.t();
  UniformCircleSampler uSamp;
  Vec3 source;

  if (!random)
    uSamp.setSampling(radius, number);

  Vec3 sourceCenter = 
      distance * beamSys.row.vz
    + offX * elOrient.row.vx
    + offY * elOrient.row.vy
    + elCenter;

  Vec3 direction  = -beamSys.row.vz;
  Ray  ray;

  for (auto i = 0; i < number || !random; ++i) {
    if (random) {
      Real sep      = radius * sqrt(.5 * (1 + RZ_URANDSIGN));
      Real angle    = RZ_URANDSIGN * M_PI;
      source        = sep * (cos(angle) * beamSys.row.vx + sin(angle) * beamSys.row.vy) + sourceCenter;
    } else {
      if (!uSamp.sample(source, beamSys, sourceCenter))
        break;
    }

    ray.origin    = source;
    ray.direction = direction;
    ray.length    = distance;
    ray.id        = id;
    dest.push_back(ray);
  }
}

void
OMModel::addElementRelativeFocusBeam(
    std::list<Ray> &dest,
    Element *element,
    unsigned int number,
    Real radius,
    Real fNum,
    Real refAperture,
    Real azimuth,
    Real elevation,
    Real offX,
    Real offY,
    Real distance,
    uint32_t id,
    bool random)
{
  const ReferenceFrame *frame;
  Real focalLength = refAperture * fNum;

  if (element->hasProperty("optical")) {
    OpticalElement *optEl = static_cast<OpticalElement *>(element);
    frame = optEl->opticalPath().m_sequence.begin()->frame;
  } else {
    frame = element->parentFrame();
  }

  Matrix3 elOrient  = frame->getOrientation();
  Vec3    elCenter  = frame->getCenter();
  Matrix3 beamSys   = Matrix3::azel(deg2rad(azimuth), deg2rad(elevation));
  Matrix3 orient    = beamSys.t();

  Vec3 sourceCenter = 
    + offX * elOrient.row.vx
    + offY * elOrient.row.vy
    + elCenter;

  Vec3 focus      = elCenter + -focalLength * beamSys.row.vz;
  Ray  ray;
  UniformCircleSampler uSamp;

  if (!random)
    uSamp.setSampling(radius, number);

  for (auto i = 0; i < number || !random; ++i) {
    Vec3 arrival;
    if (random) {
      Real sep       = radius * sqrt(.5 * (1 + RZ_URANDSIGN));
      Real angle     = RZ_URANDSIGN * M_PI;
      arrival   = elCenter + sep * (cos(angle) * beamSys.row.vx + sin(angle) * beamSys.row.vy);
    } else {
      if (!uSamp.sample(arrival, beamSys, elCenter))
        break;
    }

    Vec3 direction = (focus - arrival).normalized();

    if (fNum < 0)
      direction = -direction;

    Vec3 source   = -direction * distance + arrival;
    
    ray.origin    = source;
    ray.direction = direction;
    ray.length    = distance;
    ray.id        = id;
    dest.push_back(ray);
  }
}

void
OMModel::addFocalPlaneFocusedBeam(
    std::list<Ray> &dest,
    const ReferenceFrame *frame,
    unsigned int number,
    Real fNum,
    Real azimuth,
    Real elevation,
    Real offX,
    Real offY,
    Real distance,
    uint32_t id,
  bool random)
{
  Matrix3 elOrient  = frame->getOrientation();
  Vec3    elCenter  = frame->getCenter();
  Matrix3 beamSys   = Matrix3::azel(deg2rad(azimuth), deg2rad(elevation));
  Matrix3 orient    = beamSys.t();
  Real    radius    = .5 * distance / fabs(fNum);
  UniformCircleSampler uSamp;
  Vec3 displ;

  if (!random)
    uSamp.setSampling(radius, number);
    
  Vec3 focusLocation = 
    + offX * elOrient.row.vx
    + offY * elOrient.row.vy
    + elCenter;

  Ray  ray;

  for (auto i = 0; i < number || !random; ++i) {
    if (random) {
      Real sep       = radius * sqrt(.5 * (1 + RZ_URANDSIGN));
      Real angle     = RZ_URANDSIGN * M_PI;
      displ     = sep * (cos(angle) * beamSys.row.vx + sin(angle) * beamSys.row.vy);
    } else {
      if (!uSamp.sample(displ, beamSys, Vec3::zero()))
        break;
    }

    Vec3 origin;

    if (fNum > 0)
      origin    = focusLocation + displ + distance * beamSys.row.vz;
    else
      // Negative fNum: converging ray from below
      origin    = focusLocation + displ - distance * beamSys.row.vz;  

    Vec3 direction = (focusLocation - origin).normalized();

    ray.origin     = origin;
    ray.direction  = direction;
    ray.length     = distance;
    ray.id        = id;
    dest.push_back(ray);
  }
}

OMModel::OMModel()
{
  auto sing = Singleton::instance();
  auto factory = sing->lookupElementFactory("RayBeam");

  registerFrame(m_world = new WorldFrame("world"));

  m_beam = static_cast<RayBeamElement *>(factory->make("beam", m_world));
  registerElement(m_beam);
}

OMModel::~OMModel()
{
  for (auto frame : m_frames)
    delete frame;

  for (auto element : m_elements)
    delete element;
}
