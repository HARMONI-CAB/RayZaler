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

#include <OMModel.h>
#include <Element.h>
#include <RayBeamElement.h>
#include <CPURayTracingEngine.h>
#include <RotatedFrame.h>
#include <TranslatedFrame.h>
#include <Logger.h>
#include <sys/time.h>
#include <GenericAperture.h>
#include <Samplers/Circular.h>
#include <Samplers/Ring.h>
#include <Samplers/Point.h>
#include <Samplers/Map.h>

#define TRACE_PROGRESS_INTERVAL_MS 250

using namespace RZ;

void
BeamProperties::debug() const
{
  RZInfo("Reference: %d\n", reference);
  RZInfo("Converging: %d\n", converging);
  RZInfo("Refptr: %p\n", frame);
  RZInfo("NumRays: %d\n", numRays);
  RZInfo("Direction: %s\n", direction.toString().c_str());
  RZInfo("Offset: %s\n", offset.toString().c_str());
  RZInfo("Random: %d\n", random);
  RZInfo("Shape: %d\n", shape);
  RZInfo("Focus Z: %g\n", focusZ);
  RZInfo("Length: %g\n", length);
  RZInfo("Diameter: %g\n", diameter);
}

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

Element *
OMModel::resolveElement(std::string const &path) const
{
  const OMModel *model = this;
  Element *currElement;

  for (auto &part : path / "/") {
    if (part.empty())
      continue;
    
    currElement = nullptr;
    
    // Lookup can only be performed if we are inside a model
    if (model == nullptr)
      break;

    currElement = model->lookupElement(part);
    if (currElement == nullptr)
      break;
    
    model = currElement->nestedModel();
  }

  return currElement;
}

OpticalElement *
OMModel::resolveOpticalElement(std::string const &path) const
{
  Element *element = resolveElement(path);

  if (element == nullptr)
    return nullptr;

  if (!element->hasProperty("optical"))
    return nullptr;

  return static_cast<OpticalElement *>(element);
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
OMModel::elementHierarchy(std::string const &pfx) const
{
  std::list<std::string> keys;

  for (auto p : m_nameToElement) {
    std::string name = pfx + p.first;
    keys.push_back(name);
    if (p.second->nestedModel() != nullptr)
      keys.splice(
        keys.end(),
        p.second->nestedModel()->elementHierarchy(name + "/"));
  }

  return keys;
}

std::list<std::string>
OMModel::opticalElementHierarchy(std::string const &pfx) const
{
std::list<std::string> keys;

  for (auto p : m_nameToElement) {
    std::string name = pfx + p.first;
    if (p.second->hasProperty("optical"))
      keys.push_back(name);
    
    if (p.second->nestedModel() != nullptr)
      keys.splice(
        keys.end(),
        p.second->nestedModel()->opticalElementHierarchy(name + "/"));
  }
  
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

std::list<Element *>
OMModel::allElements() const
{
  std::list<Element *> elements;

  for (auto elem : m_elements) {
    elements.push_back(elem);
    if (elem->nestedModel() != nullptr)
      elements.splice(
        elements.end(),
        elem->nestedModel()->allElements());
  }

  return elements;
}

std::list<OpticalElement *>
OMModel::allOpticalElements() const
{
  std::list<OpticalElement *> elements;

  for (auto elem : m_elements) {
    if (elem->hasProperty("optical"))
      elements.push_back(static_cast<OpticalElement *>(elem));
    if (elem->nestedModel() != nullptr)
      elements.splice(
        elements.end(),
        elem->nestedModel()->allOpticalElements());
  }

  return elements;
}

RayBeamElement *
OMModel::beam() const
{
  return m_beam;
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
  RZError("Diffraction simulation not yet implemented");
  return false;
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
    
    tracer.setCurrentSurface(p);
    tracer.trace();

    // Check if tracer was cancelled at this point
    if (listener != nullptr && listener->cancelled())
      return false;

    // - Amplitudes are up-to date now
    tracer.transfer();

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
  BeamProperties prop;

  prop.numRays   = number;
  prop.diameter  = 2 * radius;
  prop.direction = -Matrix3::azel(deg2rad(azimuth), deg2rad(elevation)).vz();

  prop.length    = distance;
  prop.id        = id;
  prop.random    = random;

  prop.collimate();

  addBeam(dest, prop);
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
  BeamProperties prop;

  prop.setElementRelative(element);
  prop.numRays   = number;
  prop.diameter  = 2 * radius;
  prop.direction = -Matrix3::azel(deg2rad(azimuth), deg2rad(elevation)).vz();
  prop.offset    = Vec3(offX, offY, 0);
  prop.length    = distance;
  prop.id        = id;
  prop.random    = random;

  prop.collimate();
  
  addBeam(dest, prop);
}

//
// When I coded this, I didn't realize how awful this definition would be. I keep
// this only for compatibility purposes, but I will remove it before I write
// the documentation for this file. The rough idea was as follows:
//
// 1. Here, radius refers to "how wide" is the radius of the beam at the
//    arrival point.
// 2. fNum is only used to calculate a focal length w.r.t certain reference
//    aperture.
// 3. From this focal length, I calculate the focal point.
// 4. I sample the destination plane (???)
// 5. I deduce the ray origins from the focus and the sampling of the destination plane.
//
// Nope, not keeping this behavior. Sorry.
//
void
OMModel::addElementRelativeFocusBeam(
    std::list<Ray> &dest,
    Element *element,
    unsigned int number,
    Real radius, // Ignored
    Real fNum,
    Real refAperture, // Ignored. Oh my God.
    Real azimuth,
    Real elevation,
    Real offX,
    Real offY,
    Real distance,
    uint32_t id,
    bool random)
{
  BeamProperties prop;

  prop.setElementRelative(element);
  prop.converging = fNum > 0;
  prop.numRays    = number;

  prop.direction  = -Matrix3::azel(deg2rad(azimuth), deg2rad(elevation)).vz();
  prop.offset     = Vec3(offX, offY, 0);
  prop.length     = distance;
  prop.id         = id;
  prop.random     = random;

  // Adjust beam diameter to attain this fNum
  prop.setFNum(fabs(fNum), RZ::BeamDiameter);

  addBeam(dest, prop);
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
    bool random,
    Real offZ)
{
  BeamProperties prop;

  prop.setPlaneRelative(frame);
  prop.converging = fNum > 0;
  prop.numRays    = number;

  prop.direction  = -Matrix3::azel(deg2rad(azimuth), deg2rad(elevation)).vz();
  prop.offset     = Vec3(offX, offY, offZ);
  prop.length     = distance;
  prop.id         = id;
  prop.random     = random;

  // Adjust beam diameter to attain this fNum
  prop.setFNum(fabs(fNum), RZ::BeamDiameter);

  addBeam(dest, prop);
}

void
OMModel::linkWorld(ReferenceFrame *frame)
{
  WorldFrame *world = static_cast<WorldFrame *>(m_world);
  world->linkParent(frame);
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

void
OMModel::addBeam(std::list<Ray> &dest, BeamProperties const &properties)
{
  const ReferenceFrame *frame = nullptr;
  Sampler *raySampler = nullptr;
  WorldFrame worldFrame("sky");
  const OpticalElement *optEl;

  switch (properties.reference) {
    case SkyRelative:
      frame = &worldFrame;
      break;

    case ElementRelative:
      if (properties.element == nullptr)
        throw std::runtime_error("No element defined in property structure");

      if (properties.element->hasProperty("optical")) {
        optEl = static_cast<const OpticalElement *>(properties.element);
        frame = optEl->opticalPath().m_sequence.front()->frame;
      } else {
        frame = properties.element->parentFrame();
      }
      break;

    case PlaneRelative:
      if (properties.frame == nullptr)
        throw std::runtime_error("No frame defined in property structure");
      frame = properties.frame;
      break;

    default:
      throw std::runtime_error("Invalid beam reference");
  }

  // Direction at which rays arrive
  Vec3 mainDirection = frame->fromRelativeVec(properties.direction);
  Vec3 direction;
  SkySampler dirSampler(mainDirection);

  // Point towards which rays are casted
  Vec3 center    = frame->fromRelative(properties.offset);

  // Point from which the rays are casted
  Vec3 origin    = center - mainDirection * properties.length;

  // In order to sample points across the section of the beam, we need to
  // compose a 2D system that is perpendicular to the direction of the rays
  // and that is centered in the origin.
  //
  // We can achieve this by the following strategy:
  //   - Choose eV from (eX, eY) so that its projection with the direction vector is the smallest
  //   - Calculate eX' = direction x eV
  //   - Calculate eY' = eX' x direction
  //
  Vec3 eV = fabs(mainDirection * frame->eX()) < fabs(mainDirection * frame->eY())
            ? frame->eX()
            : frame->eY();
  Vec3 eXp = mainDirection.cross(eV);
  Vec3 eYp = eXp.cross(mainDirection);
  Matrix3 system = Matrix3(eXp, eYp, mainDirection).t();

  switch (properties.shape) {
    case Circular:
      raySampler = new CircularSampler();
      break;

    case Ring:
      raySampler = new RingSampler();
      break;

    case Point:
      raySampler = new PointSampler();
      break;

    case Custom:
      raySampler = new MapSampler(properties.path);
      break;

    default:
      throw std::runtime_error("Invalid beam shape");
  }

  raySampler->setRadius(.5 * properties.diameter);
  raySampler->setRandom(properties.random);

  if (!raySampler->sample(properties.numRays)) {
    delete raySampler;
    throw std::runtime_error("Failed to acquire points: beam sampler failed");
  }

  Ray ray;
  Vec3 coord;
  ray.id    = properties.id;
  ray.chief = !properties.vignetting;

  if (std::isinf(properties.focusZ)) {
    // Collimated beams are easy to calculate. Just throw some rays parallel
    // to the chief ray.

    dirSampler.setNumRays(properties.numRays);
    dirSampler.setPath(properties.objectPath);
    dirSampler.setShape(properties.objectShape);
    dirSampler.setDiameter(properties.angularDiameter);

    while (raySampler->get(coord) && dirSampler.get(direction)) {
      if (properties.objectShape != PointLike)
        origin = center - direction * properties.length;
      ray.origin    = system * coord + origin;
      ray.direction = direction;
      dest.push_back(ray);
    }
  } else {
    // Focused beams are a bit trickier, as they have this focus term
    // that determines the vertex at which free rays (i.e. with no obstacles)
    // would converge. There are two cases here:

    // - Converging beams depart from origin to center, and the focus is in
    //   [length] meters from origin, in positive chief ray direction

    // - Diverging beams depart from origin to center, and the focus is in
    //   [length] meters from origin, in negative chief ray direction

    if (properties.objectShape != PointLike) {
      delete raySampler;
      throw std::runtime_error("Cannot sample focused rays from the sky");
    }

    if (properties.converging) {
      direction = mainDirection;
      Vec3 focus     = origin + direction * (properties.length + properties.focusZ);

      while (raySampler->get(coord)) {
        ray.origin    = system * coord + origin;
        ray.direction = (focus - ray.origin).normalized();
        dest.push_back(ray);
      }
    } else {
      Vec3 focus     = origin - direction * (properties.length + properties.focusZ);

      while (raySampler->get(coord)) {
        ray.origin    = system * coord + origin;
        ray.direction = (ray.origin - focus).normalized();
        dest.push_back(ray);
      }
    }
  }
}
