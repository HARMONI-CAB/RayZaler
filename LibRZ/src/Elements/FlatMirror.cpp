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

#include <Elements/FlatMirror.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <Surfaces/Circular.h>
#include <Logger.h>

using namespace RZ;

RZ_DESCRIBE_OPTICAL_ELEMENT(FlatMirror, "Elliptic mirror with a flat surface")
{
  property("thickness",       1e-2, "Thickness of the mirror [m]");
  property("radius",        2.5e-2, "Radius of the mirror [m]");
  property("diameter",        5e-2, "Diameter of the mirror [m]");
  property("width",           5e-2, "Width of the mirror [m]");
  property("height",          5e-2, "Height of the mirror [m]");
  property("vertexRelative", false, "Positioning is relative to the vertex of the reflective surface");;
}

void
FlatMirror::recalcModel()
{
  Real backPlane, frontPlane;

  m_cylinder.setHeight(m_thickness);
  m_cylinder.setRadius(m_radius);

  m_boundary->setRadius(m_radius);
  m_boundary->setEccentricity(m_ecc);

  if (m_vertexRelative) {
    backPlane  = -m_thickness;
    frontPlane = 0;
  } else {
    backPlane  = 0;
    frontPlane = m_thickness;
  }

  m_reflectiveSurfaceFrame->setDistance(frontPlane * Vec3::eZ());
  m_baseFrame->setDistance(backPlane * Vec3::eZ());

  m_a = .5 * m_width  / m_radius;
  m_b = .5 * m_height / m_radius;

  setBoundingBox(
      Vec3(-m_width / 2, -m_height / 2, backPlane),
      Vec3(+m_width / 2, +m_height / 2, frontPlane));

  updatePropertyValue("radius",   m_radius);
  updatePropertyValue("diameter", 2 * m_radius);
  
  refreshFrames();
}

bool
FlatMirror::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "vertexRelative") {
    m_vertexRelative = static_cast<bool>(value);
    recalcModel();
  } else if (name == "radius") {
    m_radius = value;
    m_width  = 2 * m_radius;
    m_height = 2 * m_radius;
    m_ecc    = 0;
    recalcModel();
    } else if (name == "diameter") {
    m_radius = 0.5 * std::get<Real>(value);
    m_width  = 2 * m_radius;
    m_height = 2 * m_radius;
    m_ecc    = 0;
    recalcModel();
  } else if (name == "width") {
    m_width  = value;
    CircularFlatSurface::radiusEccentricity(m_radius, m_ecc, m_width, m_height);
    recalcModel();
  } else if (name == "height") {
    m_height = value;
    CircularFlatSurface::radiusEccentricity(m_radius, m_ecc, m_width, m_height);
    recalcModel();
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}


FlatMirror::FlatMirror(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_boundary = new FlatMirrorBoundary;

  m_reflectiveSurfaceFrame = new TranslatedFrame("refSurf", frame, Vec3::zero());
  m_baseFrame              = new TranslatedFrame("basePos", frame, Vec3::zero());
  m_flipFrame              = new RotatedFrame("base", m_baseFrame, Vec3::eX(), M_PI);

  pushOpticalSurface("refSurf", m_reflectiveSurfaceFrame, m_boundary);
  addPort("vertex", m_reflectiveSurfaceFrame);
  addPort("base", m_flipFrame);

  m_cylinder.setVisibleCaps(true, true);
  
  recalcModel();
}

FlatMirror::~FlatMirror()
{
  if (m_boundary != nullptr)
    delete m_boundary;

  if (m_baseFrame != nullptr)
    delete m_baseFrame;

  if (m_flipFrame != nullptr)
    delete m_flipFrame;
}

Vec3
FlatMirror::getVertex() const
{
  return m_reflectiveSurfaceFrame->getCenter();
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

  if (m_vertexRelative)
    glTranslatef(0, 0, -m_thickness);
  
  glScalef(m_a, m_b, 1);
  m_cylinder.display();

  glPopMatrix();
}
