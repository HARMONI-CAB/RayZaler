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

#include <RectangularStop.h>
#include <TranslatedFrame.h>
#include <Apertures/Rectangular.h>

using namespace RZ;

void
RectangularStop::recalcModel()
{
  if (m_borderWidth < m_width)
    m_borderWidth = 1.1 * m_width;
  
  if (m_borderHeight < m_height)
    m_borderHeight = 1.1 * m_height;

  m_processor->setWidth(m_width);
  m_processor->setHeight(m_height);

  Real verticalSpacing   = .5 * (m_borderHeight - m_height);
  Real horizontalSpacing = .5 * (m_borderWidth - m_width);

  m_hShift = .5 * (m_width  + horizontalSpacing);
  m_vShift = .5 * (m_height + verticalSpacing);

  m_vRect.setWidth(m_borderWidth);
  m_vRect.setHeight(verticalSpacing);

  m_hRect.setWidth(horizontalSpacing);
  m_hRect.setHeight(m_borderHeight);
}

bool
RectangularStop::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "borderWidth") {
    m_borderWidth = value;
    recalcModel();
  } else if (name == "borderHeight") {
    m_borderHeight = value;
    recalcModel();
  } else if (name == "width") {
    m_width = value;
    recalcModel();
  } else if (name == "height") {
    m_height = value;
    recalcModel();
  } else {
    return false;
  }

  return true;
}


RectangularStop::RectangularStop(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new RectangularStopProcessor;

  registerProperty("borderWidth",  10e-2);
  registerProperty("borderHeight", 10e-2);
  registerProperty("width",       7.5e-2);
  registerProperty("height",      7.5e-2);

  m_stopSurface = new TranslatedFrame("refSurf", frame, Vec3::zero());

  pushOpticalSurface("stopSurf", m_stopSurface, m_processor);

  recalcModel();
}

RectangularStop::~RectangularStop()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
RectangularStop::nativeMaterialOpenGL(std::string const &)
{
  GLVectorStorage vec;
  GLfloat shiny = 0;
  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.1, .1, .1));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, 0, 0));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
RectangularStop::renderOpenGL()
{
  Real eps = 1e-3 * (m_width + m_height);

  material("input.surface");
  glPushMatrix();
    glTranslatef(0, 0, -eps);
    glRotatef(180, 1, 0, 0);
    glPushMatrix();
      glTranslatef(m_hShift, 0, 0);
      m_hRect.display();
      glTranslatef(-2 * m_hShift, 0, 0);
      m_hRect.display();
    glPopMatrix();
    
    glPushMatrix();
      glTranslatef(0, m_vShift, 0);
      m_vRect.display();
      glTranslatef(0, -2 * m_vShift, 0);
      m_vRect.display();
    glPopMatrix();
  glPopMatrix();

  glPushMatrix();
    glTranslatef(0, 0, eps);
    
    material("output.surface");
    
    glPushMatrix();
      glTranslatef(m_hShift, 0, 0);
      m_hRect.display();
      glTranslatef(-2 * m_hShift, 0, 0);
      m_hRect.display();
    glPopMatrix();
    
    glPushMatrix();
      glTranslatef(0, m_vShift, 0);
      m_vRect.display();
      glTranslatef(0, -2 * m_vShift, 0);
      m_vRect.display();
    glPopMatrix();
  glPopMatrix();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
RectangularStopFactory::name() const
{
  return "RectangularStop";
}

Element *
RectangularStopFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new RectangularStop(this, name, pFrame, parent);
}
