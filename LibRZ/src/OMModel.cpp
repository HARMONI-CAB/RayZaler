#include <OMModel.h>
#include <Element.h>
#include <RayBeamElement.h>
#include <CPURayTracingEngine.h>
#include <RotatedFrame.h>
#include <TranslatedFrame.h>

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
  std::list<OpticalElement *> elements;

  if (lookupOpticalPath(name) != nullptr)
    return false;

  for (auto p : list)
    elements.push_back(lookupOpticalElementOrEx(p));

  OpticalElement *prev = nullptr;
  OpticalPath *path = nullptr;

  for (auto curr : elements) {
    if (prev == nullptr) {
      m_paths.push_back(curr->opticalPath());
      path = &m_paths.back();
    } else {
      path->plug(curr);
    }

    prev = curr;
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

bool
OMModel::trace(
        std::string const &pathName,
        std::list<Ray> const &rays,
        bool updateBeamElement,
        RayTracingProcessListener *listener,
        bool clear)
{
  CPURayTracingEngine tracer;
  unsigned int n, stages;
  const OpticalPath *path = lookupOpticalPathOrEx(pathName);
  
  m_intermediateRays.clear();

  recalculate();

  tracer.setListener(listener);

  // Clear all detectors
  if (clear)
    for (auto p : m_nameToDetector)
      p.second->clear();
  
  tracer.pushRays(rays);

  stages = path->m_sequence.size();
  n      = 0;

  for (auto p : path->m_sequence) {
    if (listener != nullptr)
      listener->stageProgress(
        PROGRESS_TYPE_TRACE,
        p.name,
        n, 
        stages);

    tracer.trace(p.frame);
    if (listener != nullptr && listener->cancelled())
      return false;

    if (updateBeamElement) {
      auto rays = tracer.getRays();
      m_intermediateRays.insert(
        m_intermediateRays.end(),
        rays.begin(),
        rays.end());
    }

    ++n;

    if (listener != nullptr)
      listener->stageProgress(
        PROGRESS_TYPE_TRANSFER,
        p.name,
        n, 
        stages);

    tracer.transfer(p.processor);

    if (listener != nullptr && listener->cancelled())
      return false;
  }

  // TODO: make beam thread-safe
  if (updateBeamElement)
    m_beam->setList(m_intermediateRays);

  return true;
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
OMModel::addSkyBeam(
  std::list<Ray> &dest,
  unsigned int number,
  Real radius,
  Real azimuth,
  Real elevation,
  Real distance)
{
  Matrix3 starSys   = Matrix3::azel(deg2rad(azimuth), deg2rad(elevation));
  Vec3 sourceCenter = distance * starSys.row.vz;
  Vec3 direction    = -starSys.row.vz;
  Ray  ray;

  for (auto i = 0; i < number; ++i) {
    Real sep      = radius * sqrt(.5 * (1 + RZ_URANDSIGN));
    Real angle    = RZ_URANDSIGN * M_PI;
    Vec3 source   = sep * (cos(angle) * starSys.row.vx + sin(angle) * starSys.row.vy) + sourceCenter;
    
    ray.origin    = source;
    ray.direction = direction;
    ray.length    = distance;

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
  Real distance)
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

  Vec3 sourceCenter = 
      distance * beamSys.row.vz
    + offX * elOrient.row.vx
    + offY * elOrient.row.vy
    + elCenter;

  Vec3 direction  = -beamSys.row.vz;
  Ray  ray;

  for (auto i = 0; i < number; ++i) {
    Real sep      = radius * sqrt(.5 * (1 + RZ_URANDSIGN));
    Real angle    = RZ_URANDSIGN * M_PI;
    Vec3 source   = sep * (cos(angle) * beamSys.row.vx + sin(angle) * beamSys.row.vy) + sourceCenter;
    
    ray.origin    = source;
    ray.direction = direction;
    ray.length    = distance;

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
