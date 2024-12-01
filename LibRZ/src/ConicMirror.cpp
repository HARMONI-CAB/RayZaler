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

#include <ConicMirror.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
ConicMirror::recalcModel()
{
  Real R2  = m_radius * m_radius;
  Real Rc  = fabs(m_rCurv);
  Real Rc2 = m_rCurv  * m_rCurv;
  bool convex = m_rCurv < 0;
  Real sigma = convex ? 1 : -1;
  
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

  m_cylinder.setHeight(m_thickness);
  m_cylinder.setCaps(&m_cap, &m_rearCap);

  m_reflectiveSurfaceFrame->setDistance(apertureZ * Vec3::eZ());
  m_aperturePort->setDistance(apertureZ * Vec3::eZ());
  m_vertexPort->setDistance((m_thickness + backPlaneZ) * Vec3::eZ());

  m_reflectiveSurfaceFrame->recalculate();
  m_aperturePort->recalculate();
  m_vertexPort->recalculate();
  
  m_processor->setRadius(m_radius);
  m_processor->setCurvatureRadius(Rc);
  m_processor->setConicConstant(m_K);
  m_processor->setConvex(convex);
  m_processor->setCenterOffset(m_x0, m_y0);

  m_cap.setHoleRadius(m_rHole);
  m_rearCap.setHoleRadius(m_rHole);

  m_hole.setRadius(m_rHole);
  m_hole.setInvertNormals(true);
  m_hole.setHeight(m_thickness);
  m_hole.setVisibleCaps(false, false);
  m_processor->setHoleRadius(m_rHole);

  setBoundingBox(
      Vec3(-m_radius, -m_radius, fmin(backPlaneZ, backPlaneZ - sigma * m_displacement)),
      Vec3(+m_radius, +m_radius, fmax(apertureZ, apertureZ   + sigma * m_displacement)));

  updatePropertyValue("focalLength", 0.5 * m_rCurv);
  updatePropertyValue("curvature",   m_rCurv);

  updatePropertyValue("radius",   m_radius);
  updatePropertyValue("diameter", 2 * m_radius);
}

bool
ConicMirror::propertyChanged(
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
    recalcModel();
  } else if (name == "diameter") {
    m_radius = .5 * static_cast<Real>(value);
    recalcModel();
  } else if (name == "focalLength") {
    m_rCurv = 2 * static_cast<Real>(value);
    recalcModel();
  } else if (name == "curvature") {
    m_rCurv = value;
    recalcModel();
  } else if (name == "hole") {
    m_rHole = value;
    recalcModel();
  } else if (name == "conic") {
    m_K = value;
    recalcModel();
  } else if (name == "x0") {
    m_x0 = value;
    recalcModel();
  } else if (name == "y0") {
    m_y0 = value;
    recalcModel();
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}

ConicMirror::ConicMirror(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new ConicMirrorProcessor;

  registerProperty("thickness",    1e-2);
  registerProperty("radius",     2.5e-2);
  registerProperty("diameter",     5e-2);
  registerProperty("curvature",   10e-2);
  registerProperty("focalLength",  5e-2);
  registerProperty("conic",          0.);
  registerProperty("hole",           0.);
  registerProperty("x0",             0.);
  registerProperty("y0",             0.);
  registerProperty("vertexRelative", false);

  m_reflectiveSurfaceFrame = new TranslatedFrame("refSurf",  frame, Vec3::zero());
  m_aperturePort           = new TranslatedFrame("aperture", frame, Vec3::zero());
  m_vertexPort             = new TranslatedFrame("vertex",   frame, Vec3::zero());

  pushOpticalSurface("refSurf", m_reflectiveSurfaceFrame, m_processor);
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
  if (m_processor != nullptr)
    delete m_processor;

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

///////////////////////////////// Factory //////////////////////////////////////
std::string
ConicMirrorFactory::name() const
{
  return "ConicMirror";
}

Element *
ConicMirrorFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new ConicMirror(this, name, pFrame, parent);
}
