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

#include <Elements/RectangularStop.h>
#include <TranslatedFrame.h>
#include <Surfaces/Rectangular.h>

using namespace RZ;

RZ_DESCRIBE_OPTICAL_ELEMENT(RectangularStop, "Rectangular aperture in a rectangular frame")
{
  property("borderWidth",  10e-2, "Horizontal size of the frame [m]");
  property("borderHeight", 10e-2, "Vertical size of the frame [m]");
  property("width",       7.5e-2, "Horizontal size of the aperture [m]");
  property("height",      7.5e-2, "Vertical size of the aperture [m]");
}

void
RectangularStop::recalcModel()
{
  if (m_borderWidth < m_width)
    m_borderWidth = 1.1 * m_width;
  
  if (m_borderHeight < m_height)
    m_borderHeight = 1.1 * m_height;

  m_boundary->setWidth(m_width);
  m_boundary->setHeight(m_height);

  Real fullWidth  = m_width + m_borderWidth;
  Real fullHeight = m_height + m_borderHeight;

  Real verticalSpacing   = .5 * (m_borderHeight - m_height);
  Real horizontalSpacing = .5 * (m_borderWidth - m_width);

  m_hShift = .5 * (m_width  + horizontalSpacing);
  m_vShift = .5 * (m_height + verticalSpacing);

  m_vRect.setWidth(m_borderWidth);
  m_vRect.setHeight(verticalSpacing);

  m_hRect.setWidth(horizontalSpacing);
  m_hRect.setHeight(m_borderHeight);

  setBoundingBox(
    Vec3(-fullWidth/2, -fullHeight/2, 0),
    Vec3(+fullWidth/2, +fullHeight/2, 0));
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
    return Element::propertyChanged(name, value);
  }

  return true;
}


RectangularStop::RectangularStop(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_boundary = new RectangularStopBoundary;
  m_stopSurface = new TranslatedFrame("refSurf", frame, Vec3::zero());

  pushOpticalSurface("stopSurf", m_stopSurface, m_boundary);
  addPort("aperture", m_stopSurface);
  
  recalcModel();
}

RectangularStop::~RectangularStop()
{
  if (m_boundary != nullptr)
    delete m_boundary;
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
