#include <Element.h>
#include <Singleton.h>

using namespace RZ;

Element::Element(std::string const &name, ReferenceFrame *pFrame, Element *parent)
{
  m_name        = name;
  m_parent      = parent;
  m_parentFrame = pFrame;
}

Element::~Element()
{
  for (auto p : m_portList)
    delete p;
}

void
Element::pushChild(Element *element)
{
  m_children.push_back(element);
}

void
Element::registerProperty(std::string const &name, PropertyValue const &dfl)
{
  m_properties[name] = dfl;
}

void
Element::refreshProperties()
{
  for (auto p : m_properties)
    propertyChanged(p.first, p.second);
}

bool
Element::addPort(
  std::string const &name,
  ReferenceFrame *frame)
{
  if (m_nameToPort.find(name) != m_nameToPort.end())
    return false;

  m_nameToPort[name] = frame;

  return true;
}

ReferenceFrame *
Element::registerPort(std::string const &name, ReferenceFrame *frame)
{
  if (!addPort(name, frame))
    return nullptr;
  
  m_portList.push_back(frame);
  
  return frame;
}

std::set<std::string>
Element::ports() const
{
  std::set<std::string> ports;
  
  for (auto p = m_nameToPort.begin(); p != m_nameToPort.end(); ++p)
    ports.insert(p->first);

  return ports;
}

std::set<std::string>
Element::properties() const
{
std::set<std::string> props;
  
  for (auto p = m_properties.begin(); p != m_properties.end(); ++p)
    props.insert(p->first);

  return props;
}

ReferenceFrame *
Element::getPortFrame(std::string const &name) const
{
  auto it = m_nameToPort.find(name);
  if (it != m_nameToPort.end())
    return it->second;

  return nullptr;
}

Element *
Element::plug(
  std::string const &portName,
  std::string const &type,
  std::string const &name)
{
  Singleton *singleton = Singleton::instance();
  ReferenceFrame *port = getPortFrame(portName);
  if (port == nullptr)
    return nullptr;

  ElementFactory *factory = singleton->lookupElementFactory(type);
  if (factory == nullptr)
    return nullptr;

  Element *element = factory->make(name, port, this);
  if (element == nullptr)
    return nullptr;

  pushChild(element);

  return element;
}

bool
Element::propertyChanged(std::string const &name, PropertyValue const &val)
{
  return true;
}

bool
Element::set(std::string const &name, PropertyValue const &val)
{
  auto it = m_properties.find(name);

  if (it == m_properties.end())
    return false;

  if (!propertyChanged(name, val))
    return false;

  it->second = val;

  return true;
}

PropertyValue
Element::get(std::string const &name) const
{
  auto it = m_properties.find(name);

  if (it == m_properties.end())
    return PropertyValue::undefined();

  return it->second;
}

void
Element::renderOpenGL()
{
  // NO-OP
}

OMModel *
Element::nestedModel() const
{
  return nullptr; // No nested models
}
