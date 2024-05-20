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

#include <ApertureStop.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
ApertureStop::recalcModel()
{
  if (m_height < 2 * m_radius)
    m_height = 2 * m_radius;
  
  if (m_width < 2 * m_radius)
    m_width = 2 * m_radius;

  m_processor->setRadius(m_radius);
  m_pinHole.setRadius(m_radius);
  m_pinHole.setHeight(m_height);
  m_pinHole.setWidth(m_width);
}

bool
ApertureStop::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "radius") {
    m_radius = value;
    recalcModel();
    return true;
  } else if (name == "width") {
    m_width = value;
    recalcModel();
    return true;
  } else if (name == "height") {
    m_height = value;
    recalcModel();
    return true;
  }

  return Element::propertyChanged(name, value);
}


ApertureStop::ApertureStop(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new ApertureStopProcessor;

  registerProperty("radius",    2.5e-2);
  registerProperty("width",     7.5e-2);
  registerProperty("height",    7.5e-2);

  m_stopSurface = new TranslatedFrame("refSurf", frame, Vec3::zero());

  addPort("aperture", m_stopSurface);
  pushOpticalSurface("stopSurf", m_stopSurface, m_processor);

  recalcModel();
}

ApertureStop::~ApertureStop()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
ApertureStop::nativeMaterialOpenGL(std::string const &)
{
  GLVectorStorage vec;
  GLfloat shiny = 0;
  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.1, .1, .1));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, 0, 0));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
ApertureStop::renderOpenGL()
{
  material("input.surface");
  m_pinHole.display();
  
  glRotatef(180, 1, 0, 0);
  
  material("output.surface");
  m_pinHole.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
ApertureStopFactory::name() const
{
  return "ApertureStop";
}

Element *
ApertureStopFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new ApertureStop(this, name, pFrame, parent);
}
