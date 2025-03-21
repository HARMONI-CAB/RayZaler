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

#include <Elements/ConicMirror.h>
#include <TranslatedFrame.h>
#include <Logger.h>

using namespace RZ;


RZ_DESCRIBE_OPTICAL_ELEMENT(ConicMirror, "Circular mirror with a surface given by a conic curve")
{
  property("thickness",      1e-2,      "Thickness of the mirror [m]");
  property("radius",         1e-1,      "Radius of the mirror [m]");
  property("diameter",       2 * 1e-1,  "Diameter of the mirror [m]");
  property("curvature",      .5,        "Radius of curvature of the mirror [m]");
  property("focalLength",    0.5 * .5,  "Focal length of the mirror [m]");
  property("conic",          0,         "Conic constant (K) of the reflective surface");
  property("hole",           0,         "Radius of the central hole [m]");
  property("x0",             0,         "X-axis offset [m]");
  property("y0",             0,         "Y-axis offset [m]");
  property("vertexRelative", false,     "Positioning is relative to the vertex of the reflective surface");
}

void
ConicMirror::recalcModel()
{
  Real R2  = m_radius * m_radius;
  Real Rc  = fabs(m_rCurv);
  Real Rc2 = m_rCurv  * m_rCurv;
  bool convex = m_rCurv < 0;
  Real sigma = convex ? 1 : -1;
  
  if (m_rHole + m_thickness > m_radius)
    m_rHole = fmax(0, m_radius - m_thickness);

  if (isZero(m_K + 1)) {
    m_displacement = .5 * R2 / m_rCurv;
    m_rHoleHeight  = .5 * m_rHole * m_rHole / m_rCurv;
  } else {
    m_displacement = (Rc - sqrt(Rc2 - (m_K + 1) * R2)) / (m_K + 1);
    m_rHoleHeight  = (Rc - sqrt(Rc2 - (m_K + 1) * m_rHoleHeight * m_rHoleHeight)) / (m_K + 1);
  }

  Real backPlaneZ, apertureZ;

  if (m_vertexRelative) {
    // Mirror is centered around vertex
    apertureZ  = -sigma * m_displacement;
    backPlaneZ = -m_thickness;
  } else {
    // Mirror starts at backplane (default)
    apertureZ  = m_thickness - sigma * m_displacement;
    backPlaneZ = 0;
  }

  m_cap.setRadius(m_radius);
  m_cap.setCurvatureRadius(Rc);
  m_cap.setConicConstant(m_K);
  m_cap.setConvex(convex);
  m_cap.setInvertNormals(false);
  m_cap.setCenterOffset(m_x0, m_y0);
  
  m_rearCap.setRadius(m_radius);
  m_rearCap.setCurvatureRadius(Rc);
  m_rearCap.setConicConstant(m_K);
  m_rearCap.setConvex(convex);
  m_rearCap.setInvertNormals(true);
  m_rearCap.setCenterOffset(m_x0, m_y0);

  m_cap.requestRecalc();
  m_rearCap.requestRecalc();

  m_cylinder.setHeight(m_thickness);
  m_cylinder.setCaps(&m_cap, &m_rearCap);

  m_reflectiveSurfaceFrame->setDistance(apertureZ * Vec3::eZ());
  m_aperturePort->setDistance(apertureZ * Vec3::eZ());
  m_vertexPort->setDistance((m_thickness + backPlaneZ) * Vec3::eZ());
  
  m_boundary->setRadius(m_radius);
  m_boundary->setCurvatureRadius(Rc);
  m_boundary->setConicConstant(m_K);
  m_boundary->setConvex(convex);
  m_boundary->setCenterOffset(m_x0, m_y0);

  m_cap.setHoleRadius(m_rHole);
  m_rearCap.setHoleRadius(m_rHole);

  m_hole.setRadius(m_rHole);
  m_hole.setInvertNormals(true);
  m_hole.setHeight(m_thickness);
  m_hole.setVisibleCaps(false, false);
  m_boundary->setHoleRadius(m_rHole);

  setBoundingBox(
      Vec3(-m_radius + m_x0, -m_radius + m_y0, fmin(backPlaneZ, backPlaneZ - sigma * m_displacement)),
      Vec3(+m_radius + m_x0, +m_radius + m_y0, fmax(apertureZ, apertureZ   + sigma * m_displacement)));

  updatePropertyValue("hole",        m_rHole);
  updatePropertyValue("focalLength", 0.5 * m_rCurv);
  updatePropertyValue("curvature",   m_rCurv);

  updatePropertyValue("radius",   m_radius);
  updatePropertyValue("diameter", 2 * m_radius);

  refreshFrames();
}

bool
ConicMirror::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness")
    m_thickness = value;
  else if (name == "vertexRelative")
    m_vertexRelative = static_cast<bool>(value);
  else if (name == "radius")
    m_radius = value;
  else if (name == "diameter")
    m_radius = .5 * static_cast<Real>(value);
  else if (name == "focalLength")
    m_rCurv = 2 * static_cast<Real>(value);
  else if (name == "curvature")
    m_rCurv = value;
  else if (name == "hole")
    m_rHole = value;
  else if (name == "conic")
    m_K = value;
  else if (name == "x0")
    m_x0 = value;
  else if (name == "y0")
    m_y0 = value;
  else
    return Element::propertyChanged(name, value);

  recalcModel();
  return true;
}

ConicMirror::ConicMirror(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_boundary = new ConicMirrorBoundary;

  m_reflectiveSurfaceFrame = new TranslatedFrame("refSurf",  frame, Vec3::zero());
  m_aperturePort           = new TranslatedFrame("aperture", frame, Vec3::zero());
  m_vertexPort             = new TranslatedFrame("vertex",   frame, Vec3::zero());

  pushOpticalSurface("refSurf", m_reflectiveSurfaceFrame, m_boundary);
  addPort("aperture", m_aperturePort);
  addPort("vertex",   m_vertexPort);

  m_cylinder.setVisibleCaps(false, false);
  m_cap.setInvertNormals(true);
  m_rearCap.setInvertNormals(false);

  refreshProperties();
  recalcModel();
}

ConicMirror::~ConicMirror()
{
  if (m_boundary != nullptr)
    delete m_boundary;

  if (m_aperturePort != nullptr)
    delete m_aperturePort;

  if (m_vertexPort != nullptr)
    delete m_vertexPort;
}

void
ConicMirror::nativeMaterialOpenGL(std::string const &)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
ConicMirror::renderOpenGL()
{
  bool convex = m_rCurv < 0;
  Real sigma = convex ? 1 : -1;
  Real dz    = m_vertexRelative ? -m_thickness : 0;

  glPushMatrix();
  material("mirror");
  glTranslatef(0, 0, dz - sigma * m_displacement);

  m_rearCap.display();
  m_cylinder.display();

  material("input.mirror");

  glTranslatef(0, 0, m_thickness);
  m_cap.display();
  glPopMatrix();

  glTranslatef(0, 0, dz - sigma * m_rHoleHeight);
  m_hole.display();
}
