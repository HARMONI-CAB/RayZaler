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

#include <ParabolicMirror.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
ParabolicMirror::recalcModel()
{
  m_displacement = .25 * m_radius * m_radius / m_flength;

  m_cap.setRadius(m_radius);
  m_cap.setFocalLength(m_flength);
  m_cap.setInvertNormals(true);
  m_cap.setCenterOffset(m_x0, m_y0);
  
  m_rearCap.setRadius(m_radius);
  m_rearCap.setFocalLength(m_flength);
  m_rearCap.setInvertNormals(false);
  m_rearCap.setCenterOffset(m_x0, m_y0);
  
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setCaps(&m_cap, &m_rearCap);

  m_processor->setRadius(m_radius);
  m_processor->setFocalLength(-m_flength);
  m_processor->setCenterOffset(m_x0, m_y0);

  m_reflectiveSurfaceFrame->setDistance((m_thickness + m_displacement) * Vec3::eZ());
  m_reflectiveSurfacePort->setDistance((m_thickness + m_displacement) * Vec3::eZ());

  m_reflectiveSurfaceFrame->recalculate();
  m_reflectiveSurfacePort->recalculate();
}

bool
ParabolicMirror::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else if (name == "flength") {
    m_flength = value;
    recalcModel();
  } else if (name == "x0") {
    m_x0 = value;
    recalcModel();
  } else if (name == "y0") {
    m_y0 = value;
    recalcModel();
  } else {
    return false;
  }

  return true;
}

ParabolicMirror::ParabolicMirror(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new ParabolicMirrorProcessor;

  registerProperty("thickness",   1e-2);
  registerProperty("radius",    2.5e-2);
  registerProperty("flength",     5e-2);
  registerProperty("x0",            0.);
  registerProperty("y0",            0.);
  
  m_reflectiveSurfaceFrame = new TranslatedFrame("refSurf", frame, Vec3::zero());
  m_reflectiveSurfacePort  = new TranslatedFrame("refPort", frame, Vec3::zero());

  pushOpticalSurface("refSurf", m_reflectiveSurfaceFrame, m_processor);
  addPort("refPort", m_reflectiveSurfacePort);

  m_cylinder.setVisibleCaps(false, false);
  m_cap.setInvertNormals(true);
  m_rearCap.setInvertNormals(false);
  
  refreshProperties();
}

ParabolicMirror::~ParabolicMirror()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
ParabolicMirror::nativeMaterialOpenGL(std::string const &)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
ParabolicMirror::renderOpenGL()
{
  material("mirror");

  glTranslatef(0, 0,  m_displacement);

  material("input.mirror");

  m_cap.display();
  m_cylinder.display();

  glTranslatef(0, 0, m_thickness);
  m_rearCap.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
ParabolicMirrorFactory::name() const
{
  return "ParabolicMirror";
}

Element *
ParabolicMirrorFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new ParabolicMirror(this, name, pFrame, parent);
}
