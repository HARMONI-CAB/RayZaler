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

#include <Element.h>
#include <Singleton.h>
#include <GLHelpers.h>
#include <OMModel.h>
#include <Logger.h>

using namespace RZ;

Element::Element(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  m_factory     = factory;
  m_name        = name;
  m_parent      = parent;
  m_parentFrame = pFrame;

  registerProperty("red",       m_red);
  registerProperty("green",     m_green);
  registerProperty("blue",      m_blue);

  registerProperty("specRed",   m_specRed);
  registerProperty("specGreen", m_specGreen);
  registerProperty("specBlue",  m_specBlue);

  registerProperty("shiny",     m_shiny);

  m_hidden = m_properties.size();
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
  m_sortedProperties.push_back(name);
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

std::vector<std::string>
Element::sortedProperties() const
{
  return m_sortedProperties;
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
  Real value = val;

  if (name == "red")
    m_red = value;
  else if (name == "green")
    m_green = value;
  else if (name == "blue")
    m_blue = value;
  else if (name == "specRed")
    m_specRed = value;
  else if (name == "specGreen")
    m_specGreen = value;
  else if (name == "specBlue")
    m_specBlue = value;
  else if (name == "shiny")
    m_shiny = value;
  else 
    return false;

  return true;
}

bool
Element::set(std::string const &name, PropertyValue const &val)
{
  auto it = m_properties.find(name);

  if (it == m_properties.end())
    return false;

  if (!propertyChanged(name, val)) {
    RZWarning(
      "Element %s (%s): cannot set %s to %g\n",
      this->name().c_str(),
      factory()->name().c_str(),
      name.c_str(),
      (Real) val);
    return false;
  }

  it->second = val;

  return true;
}

PropertyValue
Element::get(std::string const &name) const
{
  auto it = m_properties.find(name);

  if (it == m_properties.end()) {
    RZWarning(
      "Element %s (%s): property `%s' does not exist\n",
      this->name().c_str(),
      factory()->name().c_str(),
      name.c_str());
    return PropertyValue::undefined();
  }

  return it->second;
}

void
Element::material(std::string const &role)
{
  // In selection,we override lighting
  if (m_selected) {
    GLVectorStorage vec;
    GLfloat shiny = 128;

    if (role.rfind("input.") == 0) {
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(0, 1, 0));
      glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0, .25, 0));
    } else if (role.rfind("output.") == 0) {
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(1, 0, 0));
      glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(.25, 0, 0));
    } else { 
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(1, 1, 0));
      glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(.25, .25, 0));
    }

    glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
    glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
  } else {
    nativeMaterialOpenGL(role);
  }
}

void
Element::setSelected(bool selected)
{
  auto nested = nestedModel();

  m_selected = selected;

  // For nested models, propgate selection
  if (nested != nullptr)
    for (auto p : nested->elementList())
      p->setSelected(selected);
}

void
Element::setVisible(bool visible)
{
  m_visible = visible;
}

void
Element::enterOpenGL()
{
  
}

void
Element::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  GLfloat shiny = m_shiny;

  glMaterialfv(GL_FRONT, GL_AMBIENT,  vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(m_specRed, m_specGreen, m_specBlue));
  glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(m_red, m_green, m_blue));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
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

GenericCompositeModel *
Element::nestedCompositeModel() const
{
  return nullptr; // No nested composite models
}

