//
//  Copyright (c) 2025 Gonzalo José Carracedo Carballal
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

#include <Elements/BirefringentPrism.h>
#include <TranslatedFrame.h>

using namespace RZ;

#define PRISM_DEFAULT_LENGTH 5e-2
#define PRISM_DEFAULT_WIDTH  3e-2
#define PRISM_DEFAULT_HEIGHT 1e-2


RZ_DESCRIBE_OPTICAL_ELEMENT(BirefringentPrism, "Rectangular prism made of a birefringent material")
{
  property("length",  PRISM_DEFAULT_LENGTH, "Length (X dimension) of the prism [m]");
  property("width",   PRISM_DEFAULT_WIDTH,  "Width (Y dimension) of the prism [m]");
  property("height",  PRISM_DEFAULT_HEIGHT, "Height (Z dimension) of the prism [m]");
  property("no",                       1.1, "Ordinary refractive index");
  property("ne",                       1.5, "Extraordinary refractive index");
  property("ax",                         1, "X component of the optical axis");
  property("ay",                         0, "Y component of the optical axis");
  property("az",                         0, "Z component of the optical axis");
}

void
BirefringentPrism::recalcMedium()
{
  m_inputBoundary->setMedia(nullptr, &m_glass);
  m_outputBoundary->setMedia(&m_glass, nullptr);
}

void
BirefringentPrism::recalcModel()
{
  m_inputBoundary->setWidth(m_length);
  m_inputBoundary->setHeight(m_width);
  m_inputBoundary->setMedia(nullptr, &m_glass);

  m_outputBoundary->setWidth(m_length);
  m_outputBoundary->setHeight(m_width);
  m_outputBoundary->setMedia(&m_glass, nullptr);
  
  // Intercept surfaces
  m_inputFrame->setDistance(+.5 * m_height * Vec3::eZ());
  m_outputFrame->setDistance(-.5 * m_height * Vec3::eZ());

  setBoundingBox(
      Vec3(-m_length/2, -m_width/2, -m_height/2),
      Vec3(+m_length/2, +m_width/2, +m_height/2));
  
  refreshFrames();
}

bool
BirefringentPrism::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "length") {
    m_length = value;
    recalcModel();
  } else if (name == "width") {
    m_width = value;
    recalcModel();
  } else if (name == "height") {
    m_height = value;
    recalcModel();
  } else if (name == "no") {
    m_glass.no = value;
    recalcMedium();
  } else if (name == "ne") {
    m_glass.ne = value;
    recalcMedium();
  } else if (name == "ax") {
    m_axis[0] = value;
    m_glass.axis = Vec3(m_axis).normalized();
    recalcMedium();
  } else if (name == "ay") {
    m_axis[1] = value;
    m_glass.axis = Vec3(m_axis).normalized();
    recalcMedium();
  } else if (name == "az") {
    m_axis[2] = value;
    m_glass.axis = Vec3(m_axis).normalized();
    recalcMedium();
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}

BirefringentPrism::BirefringentPrism(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_inputBoundary  = new RectangularWindowBoundary;
  m_outputBoundary = new RectangularWindowBoundary;
  
  m_inputFrame  = new TranslatedFrame("inputSurf",  frame, Vec3::zero());
  m_outputFrame = new TranslatedFrame("outputSurf", frame, Vec3::zero());

  m_glass.type  = EMMediumUniaxial;
  m_glass.frame = frame;
  m_glass.no    = 1.1;
  m_glass.ne    = 1.5;
  m_glass.axis  = Vec3::eX();

  pushOpticalSurface("inputFace",  m_inputFrame,  m_inputBoundary);
  m_inputBoundary->setMedia(nullptr, &m_glass);

  pushOpticalSurface("outputFace", m_outputFrame, m_outputBoundary);
  m_outputBoundary->setMedia(&m_glass, nullptr);

  addPort("inputPort", m_inputFrame);
  addPort("outputPort", m_outputFrame);
  
  refreshProperties();
}

BirefringentPrism::~BirefringentPrism()
{
  if (m_inputBoundary != nullptr)
    delete m_inputBoundary;

  if (m_outputBoundary != nullptr)
    delete m_outputBoundary;
}

void
BirefringentPrism::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.5, .5, .5));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, .75));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
BirefringentPrism::renderOpenGL()
{
  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_LINE_BIT);
    material("prism");

    glPushMatrix();
    glScalef(m_length, m_width, m_height);
    GLCube(1, false);
    glPopMatrix();
  glPopAttrib();
}
