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
#include <Surfaces/Conic.h>
#include <TranslatedFrame.h>
#include <Logger.h>
#include <SurfaceShape.h>

using namespace RZ;


RZ_DESCRIBE_OPTICAL_ELEMENT(ConicMirror, "Circular mirror with a surface given by a conic curve")
{
  property("thickness",      1e-2,      "Thickness of the mirror [m]");
  property("radius",         1e-1,      "Radius of the mirror [m]");
  property("diameter",       2 * 1e-1,  "Diameter of the mirror [m]");
  property("curvature",      0.5,       "Radius of curvature of the mirror [m]");
  property("focalLength",    0.5 * .5,  "Focal length of the mirror [m]");
  property("conic",          0.0,       "Conic constant (K) of the reflective surface");
  property("hole",           0.0,       "Radius of the central hole [m]");
  property("x0",             0.0,       "X-axis offset [m]");
  property("y0",             0.0,       "Y-axis offset [m]");
  property("vertexRelative", false,     "Positioning is relative to the vertex of the reflective surface");
  property("apertureType", "elliptical", "Shape of the conic mirror.");
  property("apertureHeight", 2.5e-2, "Height of the conic mirror.");
  property("apertureWidth",  2.5e-2, "Width of the conic mirror.");
}

GLAbstractCap *cap, *rearCap;

void
ConicMirror::recalcModel()
{
  //Real R2  = m_radius * m_radius;
  Real R = 2.5e-2;
  Real R2 = R * R;
  if (m_apertureType == Elliptical) {
    R = fmax(m_apertureHeight, m_apertureWidth);
    R2 = R * R;
  } else if (m_apertureType == Rectangular) {
    R = sqrt(m_apertureHeight * m_apertureHeight + m_apertureWidth * m_apertureWidth);
    R2 = R * R;
  }
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
  Real center = sqrt(m_x0 * m_x0 + m_y0 * m_y0);
  Real zPlus  = m_boundary->surfaceShape<ConicSurface>()->z(center + R);
  Real zMinus = m_boundary->surfaceShape<ConicSurface>()->z(fmax(0, center - R));


  if (m_vertexRelative) {
    // Mirror is centered around vertex
    apertureZ  = -sigma * m_displacement;
    backPlaneZ = -m_thickness;
  } else {
    // Mirror starts at backplane (default)
    apertureZ  = m_thickness - sigma * m_displacement;
    backPlaneZ = 0;
  }

  m_vertex = RZ::Vec3(
    m_x0,
    m_y0,
    apertureZ + m_boundary->surfaceShape<ConicSurface>()->z(center));

  if (m_apertureType == Elliptical) {
      m_ellipCap.setWidth(m_apertureWidth);
      m_ellipCap.setHeight(m_apertureHeight);
      m_ellipCap.setCurvatureRadius(Rc);
      m_ellipCap.setConicConstant(m_K);
      m_ellipCap.setConvex(convex);
      m_ellipCap.setInvertNormals(false);
      m_ellipCap.setCenterOffset(m_x0, m_y0);
      m_ellipCap.setHoleRadius(m_rHole);
      m_ellipCap.requestRecalc();

      m_rearEllipCap.setWidth(m_apertureWidth);
      m_rearEllipCap.setHeight(m_apertureHeight);
      m_rearEllipCap.setCurvatureRadius(Rc);
      m_rearEllipCap.setConicConstant(m_K);
      m_rearEllipCap.setConvex(convex);
      m_rearEllipCap.setInvertNormals(true);
      m_rearEllipCap.setCenterOffset(m_x0, m_y0);
      m_rearEllipCap.setHoleRadius(m_rHole);
      m_rearEllipCap.requestRecalc();
      cap = &m_ellipCap;
      rearCap = &m_rearEllipCap;
    } else if (m_apertureType == Rectangular) {
      m_rectCap.setWidth(m_apertureWidth);
      m_rectCap.setHeight(m_apertureHeight);
      m_rectCap.setCurvatureRadius(Rc);
      m_rectCap.setConicConstant(m_K);
      m_rectCap.setConvex(convex);
      m_rectCap.setInvertNormals(false);
      m_rectCap.setCenterOffset(m_x0, m_y0);
      m_rectCap.setHoleRadius(m_rHole);
      m_rectCap.requestRecalc();

      m_rearRectCap.setWidth(m_apertureWidth);
      m_rearRectCap.setHeight(m_apertureHeight);
      m_rearRectCap.setCurvatureRadius(Rc);
      m_rearRectCap.setConicConstant(m_K);
      m_rearRectCap.setConvex(convex);
      m_rearRectCap.setInvertNormals(true);
      m_rearRectCap.setCenterOffset(m_x0, m_y0);
      m_rearRectCap.setHoleRadius(m_rHole);
      m_rearRectCap.requestRecalc();
      cap = &m_rectCap;
      rearCap = &m_rearRectCap;
    }

  m_cylinder.setHeight(m_thickness);
  m_cylinder.setCaps(cap, rearCap);

  m_reflectiveSurfaceFrame->setDistance(apertureZ * Vec3::eZ());
  m_aperturePort->setDistance(apertureZ * Vec3::eZ());
  m_vertexPort->setDistance((m_thickness + backPlaneZ) * Vec3::eZ());
  
  m_boundary->setRadius(m_radius);
  m_boundary->setCurvatureRadius(Rc);
  m_boundary->setConicConstant(m_K);
  m_boundary->setConvex(convex);
  m_boundary->setCenterOffset(m_x0, m_y0);
  m_boundary->setApertureHeight(m_apertureHeight);
  m_boundary->setApertureWidth(m_apertureWidth);
  m_boundary->setApertureType(m_apertureType);

  m_hole.setRadius(m_rHole);
  m_hole.setInvertNormals(true);
  m_hole.setHeight(m_thickness);
  m_hole.setVisibleCaps(false, false);
  m_boundary->setHoleRadius(m_rHole);

  Real minZ = backPlaneZ + fmin(zPlus, zMinus);
  Real maxZ = apertureZ + fmax(zPlus, zMinus);

  setBoundingBox(
      Vec3(-m_apertureWidth + m_x0, -m_apertureHeight + m_y0, minZ),
      Vec3(+m_apertureWidth + m_x0, +m_apertureHeight + m_y0, maxZ));

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
  else if (name == "apertureType") {
    std::string type = std::get<std::string>(value);
    
    if (type == "elliptical") {
      m_apertureType = Elliptical;
    } else if (type == "rectangular") {
      m_apertureType = Rectangular;
    } else {
      m_apertureType = Elliptical;
      RZWarning("Only valid types are: elliptical and rectangular. Type has been set as elliptical as default.");
    }
  } else if (name == "apertureWidth") {
    m_apertureWidth = value;
    } else if (name == "apertureHeight") {
    m_apertureHeight = value;
  } else
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

Vec3
ConicMirror::getVertex() const
{
  return parentFrame()->fromRelative(m_vertex);
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

  rearCap->display();
  m_cylinder.display();

  material("input.mirror");

  glTranslatef(0, 0, m_thickness);
  cap->display();
  glPopMatrix();

  glTranslatef(0, 0, dz - sigma * m_rHoleHeight);
  m_hole.display();
}
