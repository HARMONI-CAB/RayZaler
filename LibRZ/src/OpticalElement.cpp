#include <OpticalElement.h>

using namespace RZ;

OpticalPath &
OpticalPath::plug(OpticalElement *element)
{
  OpticalPath copy = element->opticalPath();

  m_sequence.insert(
    m_sequence.end(),
    copy.m_sequence.begin(),
    copy.m_sequence.end());

  return *this;
}

OpticalPath
OpticalElement::opticalPath() const
{
  OpticalPath path;

  for (auto p : m_internalPath)
    path.m_sequence.push_back(p);

  return path;
}

OpticalPath
OpticalElement::plug(OpticalElement *newElement) const
{
  return opticalPath().plug(newElement);
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
}

OpticalElement::OpticalElement(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *parentFrame,
  Element *parent)
  : Element(factory, name, parentFrame, parent)
{
  registerProperty("optical", true);
}

OpticalElement::~OpticalElement()
{
  for (auto p : m_surfaceFrames)
    delete p;
}
