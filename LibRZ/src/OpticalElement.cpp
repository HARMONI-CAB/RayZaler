#include <OpticalElement.h>

using namespace RZ;

OpticalPath &
OpticalPath::plug(OpticalElement *element, std::string const &name)
{
  OpticalPath copy = element->opticalPath(name);

  m_sequence.insert(
    m_sequence.end(),
    copy.m_sequence.begin(),
    copy.m_sequence.end());

  return *this;
}

OpticalPath
OpticalElement::opticalPath(std::string const &name) const
{
  OpticalPath path;

  if (name.size() == 0)
    for (auto p : m_internalPath)
      path.m_sequence.push_back(p);
  else
    throw std::runtime_error("Unknown optical path `" + name + "'");
  
  return path;
}

OpticalPath
OpticalElement::plug(OpticalElement *newElement, std::string const &name) const
{
  return opticalPath().plug(newElement, name);
}

const std::list<const OpticalSurface *> &
OpticalElement::opticalSurfaces() const
{
  return m_surfaceList;
}

void
OpticalElement::pushOpticalSurface(
  std::string name,
  ReferenceFrame *frame,
  const RayTransferProcessor *proc)
{
  OpticalSurface surface;

  m_surfaceFrames.push_back(frame);

  surface.name      = name;
  surface.frame     = frame;
  surface.processor = proc;
  surface.parent    = this;

  m_internalPath.push_back(surface);

  auto last = &m_internalPath.back();

  m_surfaceList.push_back(last);
}

OpticalElement::OpticalElement(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *parentFrame,
  Element *parent)
  : Element(factory, name, parentFrame, parent)
{
  registerProperty("optical", true);

  ++m_hidden;
}

OpticalElement::~OpticalElement()
{
  for (auto p : m_surfaceFrames)
    delete p;
}
