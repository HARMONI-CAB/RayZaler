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

#include <FlatMirror.h>
#include <TranslatedFrame.h>
#include <Apertures/Circular.h>
#include <Logger.h>

using namespace RZ;

void
FlatMirror::recalcModel()
{
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setRadius(m_radius);

  m_processor->setRadius(m_radius);
  m_processor->setEccentricity(m_ecc);
  m_reflectiveSurfaceFrame->setDistance(m_thickness * Vec3::eZ());

  m_a = .5 * m_width  / m_radius;
  m_b = .5 * m_height / m_radius;

  setBoundingBox(
      Vec3(-m_width / 2, -m_height / 2, 0),
      Vec3(+m_width / 2, +m_height / 2, +m_thickness));
}

bool
FlatMirror::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "radius") {
    m_radius = value;
    m_width  = 2 * m_radius;
    m_height = 2 * m_radius;
    m_ecc    = 0;
    recalcModel();
  } else if (name == "width") {
    m_width  = value;
    CircularAperture::radiusEccentricity(m_radius, m_ecc, m_width, m_height);
    recalcModel();
  } else if (name == "height") {
    m_height = value;
    CircularAperture::radiusEccentricity(m_radius, m_ecc, m_width, m_height);
    recalcModel();
  } else {
    return false;
  }

  return true;
}


FlatMirror::FlatMirror(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new FlatMirrorProcessor;

  registerProperty("thickness",   1e-2);
  registerProperty("radius",    2.5e-2);
  registerProperty("width",       5e-2);
  registerProperty("height",      5e-2);

  m_reflectiveSurfaceFrame = new TranslatedFrame("refSurf", frame, Vec3::zero());

  pushOpticalSurface("refSurf", m_reflectiveSurfaceFrame, m_processor);

  m_cylinder.setVisibleCaps(true, true);
  
  recalcModel();
}

FlatMirror::~FlatMirror()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
FlatMirror::nativeMaterialOpenGL(std::string const &)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
FlatMirror::renderOpenGL()
{
  material("");
  
  glPushMatrix();

  glScalef(m_a, m_b, 1);
  m_cylinder.display();

  glPopMatrix();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
FlatMirrorFactory::name() const
{
  return "FlatMirror";
}

Element *
FlatMirrorFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new FlatMirror(this, name, pFrame, parent);
}
