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

#include <Elements/CircularWindow.h>
#include <TranslatedFrame.h>

using namespace RZ;

RZ_DESCRIBE_OPTICAL_ELEMENT(CircularWindow, "Circular window with thickness and a refractive index")
{
  property("thickness",   1e-2, "Thickness of the window [m]");
  property("radius",    2.5e-2, "Radius of the window [m]");
  property("diameter",    5e-2, "Diameter of the window [m]");
  property("n",            1.5, "Refractive index of the window");
}

void
CircularWindow::recalcModel()
{
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setRadius(m_radius);

  m_inputBoundary->setRadius(m_radius);
  m_inputBoundary->setRefractiveIndex(1, m_mu);

  m_outputBoundary->setRadius(m_radius);
  m_outputBoundary->setRefractiveIndex(m_mu, 1);

  // Intercept surfaces
  m_inputFrame->setDistance(-.5 * m_thickness * Vec3::eZ());
  m_outputFrame->setDistance(+.5 * m_thickness * Vec3::eZ());

  setBoundingBox(
      Vec3(-m_radius, -m_radius, -m_thickness/2),
      Vec3(+m_radius, +m_radius, +m_thickness/2));
  
  updatePropertyValue("radius",   m_radius);
  updatePropertyValue("diameter", 2 * m_radius);

  refreshFrames();
}

bool
CircularWindow::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else if (name == "diameter") {
    m_radius = 0.5 * static_cast<Real>(value);
    recalcModel();
  } else if (name == "n") {
    m_mu = value;
    recalcModel();
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}

CircularWindow::CircularWindow(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_inputBoundary  = new CircularWindowBoundary;
  m_outputBoundary = new CircularWindowBoundary;
  
  m_inputFrame  = new TranslatedFrame("inputSurf",  frame, Vec3::zero());
  m_outputFrame = new TranslatedFrame("outputSurf", frame, Vec3::zero());

  pushOpticalSurface("inputFace",  m_inputFrame,  m_inputBoundary);
  pushOpticalSurface("outputFace", m_outputFrame, m_outputBoundary);

  addPort("inputPort", m_inputFrame);
  addPort("outputPort", m_outputFrame);
  
  m_cylinder.setVisibleCaps(true, true);
  
  refreshProperties();
}

CircularWindow::~CircularWindow()
{
  if (m_inputBoundary != nullptr)
    delete m_inputBoundary;

  if (m_outputBoundary != nullptr)
    delete m_outputBoundary;
}

void
CircularWindow::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
CircularWindow::renderOpenGL()
{
  glTranslatef(0, 0, -.5 * m_thickness);

  material("lens");
  m_cylinder.display();
}
