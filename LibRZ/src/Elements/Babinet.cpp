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

#include <Elements/Babinet.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <Logger.h>

using namespace RZ;

#define BABINET_DEFAULT_WIDTH     5e-2
#define BABINET_DEFAULT_HEIGHT    5e-2
#define BABINET_DEFAULT_THICKNESS 1e-2

RZ_DESCRIBE_OPTICAL_ELEMENT(Babinet, "Babinet compensator")
{
  property("width",     BABINET_DEFAULT_WIDTH,     "Length (X dimension) of the compensator [m]");
  property("height",    BABINET_DEFAULT_HEIGHT,    "Height (Y dimension) of the compensator [m]");
  property("thickness", BABINET_DEFAULT_THICKNESS, "Thickness (Z dimension) of the compensator [m]");
  property("no",                       1.1, "Ordinary refractive index");
  property("ne",                       1.5, "Extraordinary refractive index");
  property("angle",                      0, "Tilt angle of the wedges");
}

void
Babinet::recalcMedium()
{
  m_inputBoundary->setMedia(nullptr, &m_inputCrystal);
  m_innerBoundary->setMedia(&m_inputCrystal, &m_outputCrystal);
  m_outputBoundary->setMedia(&m_outputCrystal, nullptr);
}

void
Babinet::recalcModel()
{
  auto angle = deg2rad(m_angle);         // Wedge angle [rad]
  auto halfThickness = m_thickness / 2;
  auto maxAngle = atan2(halfThickness, m_height);
  Real dz;

  if (fabs(angle) > maxAngle) {
    RZWarning(
      "Wedge angle %gº is out of bounds (max = ±%gº)",
      m_angle,
      rad2deg(maxAngle));

    dz = angle > 0 ? halfThickness : -halfThickness;
  } else {
    dz = sin(angle) * m_height / 2;   // Wedge displacement
  }

  m_h0 = halfThickness - dz;
  m_h1 = halfThickness + dz;

  m_slopeLength = sqrt(m_height * m_height  + 4 * dz * dz);

  m_inputBoundary->setWidth(m_width);
  m_inputBoundary->setHeight(m_height);

  m_innerBoundary->setWidth(m_width);
  m_innerBoundary->setHeight(m_slopeLength);

  m_outputBoundary->setWidth(m_width);
  m_outputBoundary->setHeight(m_height);
  
  m_wedge.setWidth(m_width);
  m_wedge.setLength(m_height); // Intentional
  m_wedge.setHeights(m_h0, m_h1);

  // Intercept surfaces
  m_inputFrame->setDistance(+halfThickness * Vec3::eZ());
  m_innerFrame->setAngle(angle);
  m_outputFrame->setDistance(-halfThickness * Vec3::eZ());

  setBoundingBox(
      Vec3(-m_width/2, -m_height/2, -m_thickness/2),
      Vec3(+m_width/2, +m_height/2, +m_thickness/2));
  
  refreshFrames();
  recalcMedium();
}

bool
Babinet::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "width") {
    m_width = value;
    recalcModel();
  } else if (name == "height") {
    m_height = value;
    recalcModel();
  } else if (name == "angle") {
    m_angle = value;
    recalcModel();
  } else if (name == "no") {
    m_inputCrystal.no  = value;
    m_outputCrystal.no = value;
    recalcMedium();
  } else if (name == "ne") {
    m_inputCrystal.ne  = value;
    m_outputCrystal.ne = value;
    recalcMedium();
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}

Babinet::Babinet(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_inputBoundary  = new RectangularWindowBoundary;
  m_innerBoundary  = new RectangularWindowBoundary;
  m_outputBoundary = new RectangularWindowBoundary;
  
  m_inputFrame  = new TranslatedFrame("inputSurf",  frame, Vec3::zero());
  m_innerFrame  = new    RotatedFrame("innerSurf",  frame, Vec3::eX(), 0);
  m_outputFrame = new TranslatedFrame("outputSurf", frame, Vec3::zero());

  m_inputCrystal.type  = EMMediumUniaxial;
  m_inputCrystal.frame = frame;
  m_inputCrystal.no    = 1.1;
  m_inputCrystal.ne    = 1.5;
  m_inputCrystal.axis  = Vec3::eX();

  m_outputCrystal.type  = EMMediumUniaxial;
  m_outputCrystal.frame = frame;
  m_outputCrystal.no    = 1.1;
  m_outputCrystal.ne    = 1.5;
  m_outputCrystal.axis  = Vec3::eY();

  pushOpticalSurface("inputFace",  m_inputFrame,  m_inputBoundary);
  pushOpticalSurface("innerFace",  m_innerFrame , m_innerBoundary);
  pushOpticalSurface("outputFace", m_outputFrame, m_outputBoundary);

  addPort("inputPort",  m_inputFrame);
  addPort("innerPort",  m_innerFrame);
  addPort("outputPort", m_outputFrame);
  
  recalcModel();
  refreshProperties();
}

Babinet::~Babinet()
{
  if (m_inputBoundary != nullptr)
    delete m_inputBoundary;

  if (m_innerBoundary != nullptr)
    delete m_innerBoundary;

  if (m_outputBoundary != nullptr)
    delete m_outputBoundary;
}

void
Babinet::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));

  if (role == "inWedge") 
    glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.5, .75, .75));
  else if (role == "outWedge") 
    glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.5, .5, .5));

  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, .75));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
Babinet::renderOpenGL()
{
  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_LINE_BIT);
    glPushMatrix();
      glRotatef(90, 0, 0, 1);
      material("inWedge");
      m_wedge.display();

      material("outWedge");
      glRotatef(180, 0, 1, 0);
      m_wedge.display();
    glPopMatrix();
  glPopAttrib();
}
