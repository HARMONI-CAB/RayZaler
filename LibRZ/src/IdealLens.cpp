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

#include <IdealLens.h>
#include <RayProcessors/IdealLens.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
IdealLens::recalcModel()
{
  m_cylinder.setHeight(0);
  m_cylinder.setRadius(m_radius);

  m_processor->setRadius(m_radius);
  m_processor->setFocalLength(m_fLen);

  m_frontFocalPlane->setDistance(+m_fLen * Vec3::eZ());
  m_backFocalPlane->setDistance(-m_fLen * Vec3::eZ());
  m_objectPlane->setDistance(+2 * m_fLen * Vec3::eZ());
  m_imagePlane->setDistance(-2 * m_fLen * Vec3::eZ());
  
  setBoundingBox(
      Vec3(-m_radius, -m_radius, 0),
      Vec3(+m_radius, +m_radius, 0));

  updatePropertyValue("radius",   m_radius);
  updatePropertyValue("diameter", 2 * m_radius);

  refreshFrames();
}

bool
IdealLens::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else if (name == "diameter") {
    m_radius = 0.5 * static_cast<Real>(value);
    recalcModel();
  } else if (name == "focalLength") {
    m_fLen = value;
    recalcModel();
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}

IdealLens::IdealLens(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor  = new IdealLensProcessor;

  registerProperty("radius",    2.5e-2);
  registerProperty("diameter",    5e-2);
  registerProperty("focalLength",   1.);
  
  m_inputFrame  = new TranslatedFrame("apertureFrame",  frame, Vec3::zero());

  pushOpticalSurface("lensSurface",  m_inputFrame,  m_processor);

  addPort("aperture", m_inputFrame);
  
  m_frontFocalPlane  = new TranslatedFrame("frontFocalPlane", frame, Vec3::zero());
  m_backFocalPlane   = new TranslatedFrame("backFocalPlane", frame, Vec3::zero());

  m_objectPlane      = new TranslatedFrame("objectPlane", frame, Vec3::zero());
  m_imagePlane       = new TranslatedFrame("imagePlane", frame, Vec3::zero());

  addPort("frontFocalPlane",  m_frontFocalPlane);
  addPort("backFocalPlane",   m_backFocalPlane);
  addPort("objectPlane",      m_objectPlane);
  addPort("imagePlane",       m_imagePlane);

  m_cylinder.setVisibleCaps(true, true);
  
  refreshProperties();
}

IdealLens::~IdealLens()
{
  if (m_processor != nullptr)
    delete m_processor;

  if (m_frontFocalPlane != nullptr)
    delete m_frontFocalPlane;
  
  if (m_backFocalPlane != nullptr)
    delete m_backFocalPlane;
  
  if (m_objectPlane != nullptr)
    delete m_objectPlane;
  
  if (m_imagePlane != nullptr)
    delete m_imagePlane;
}

void
IdealLens::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
IdealLens::renderOpenGL()
{
  material("lens");
  m_cylinder.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
IdealLensFactory::name() const
{
  return "IdealLens";
}

Element *
IdealLensFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new IdealLens(this, name, pFrame, parent);
}
