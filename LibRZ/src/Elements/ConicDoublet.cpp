//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//  Copyright (c) 2025 Pablo Álvarez Martín
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

#include <Elements/ConicDoublet.h>
#include <TranslatedFrame.h>
#include <Logger.h>
#include <Surfaces/Conic.h>
#include <LensHelpers.h>

using namespace RZ;

RZ_DESCRIBE_OPTICAL_ELEMENT(ConicDoublet, "Lens with surfaces given by conic curves")
{
  property("thickness1",       2e-2, "Thickness of the first lens [m]");
  property("edgeThickness1",   1e-2, "Thickness of side of the lens [m]");
  property("thickness2",       2e-2, "Thickness of the second lens [m]");
  property("edgeThickness2",   1e-2, "Thickness of side of the lens [m]");
  property("radius",         2.5e-2, "Radius of the conic lense doublet [m]");
  property("diameter",         5e-2, "Diameter of the conic lens doublet [m]"); 
  property("x0",                0.0, "X-axis offset [m]");
  property("y0",                0.0, "Y-axis offset [m]");
  property("n1",                1.5, "Refractive index");
  property("n2",                1.5, "Refractive index");

  property("frontCurvature",   1e-1, "Radius of curvature of the front surface [m]");
  property("frontConic",        0.0, "Conic constant (K) of the front surface");

  property("middleCurvature",  1e-1, "Radius of curvature of the middle surface [m]");
  property("middleConic",       0.0, "Conic constant (K) of the middle surface");

  property("backCurvature",    1e-1, "Radius of curvature of the back surface [m]");
  property("backConic",         0.0, "Conic constant (K) of the back surface");
  
  property("vertexRelative",  false, "The element is placed around a vertex");
  property("referenceVertex",     0, "Surface index where the reference vertex is");
}

void
ConicDoublet::recalcModel()
{

  LensSurfaceProperties surf[3];
  
  Real frontPlaneZ, middlePlaneZ, backPlaneZ;

  // Calculate properties of both surfaces.
  for (auto i = 0; i < 3; ++i)
    surf[i].setProperties(m_rCurv[i], m_K[i], m_radius, m_x0, m_y0);
  
  if (!adjustThickness(
    m_edgeThickness1,
    m_thickness1,
    m_fromEdge1,
    surf[0].sigma, 
    surf[0].displacement,
    surf[1].sigma, 
    surf[1].displacement))
     RZWarning("Invalid lens geometry: negative thickness or edge thickness in the first lens.\n");
  
  if (!adjustThickness(
    m_edgeThickness2,
    m_thickness2,
    m_fromEdge2,
    surf[1].sigma, 
    surf[1].displacement,
    surf[2].sigma, 
    surf[2].displacement))
     RZWarning("Invalid lens geometry: negative thickness or edge thickness in the second lens.\n");

  if (m_vertexRelative) {
    frontPlaneZ = -surf[0].sigma * surf[0].displacement;

    if (m_referenceVtx > 0)
      frontPlaneZ += m_thickness1;
    if (m_referenceVtx > 1)
      frontPlaneZ += m_thickness2;
  } else {
    frontPlaneZ = .5 * (m_edgeThickness1 + m_edgeThickness2);
  }
  middlePlaneZ = frontPlaneZ - m_edgeThickness1;
  backPlaneZ = middlePlaneZ - m_edgeThickness2;
  
  const Real Rmax1 = LensSurfaceProperties::Rmax(surf[0], surf[1], m_edgeThickness1);
  const Real Rmax2 = LensSurfaceProperties::Rmax(surf[1], surf[2], m_edgeThickness2);
  const Real Rmax = std::min(Rmax1, Rmax2);

  Real zSupVal = 0;
  Real zInfVal = 0;

  if (!LensSurfaceProperties::zLimits(
    zSupVal,
    zInfVal,
    surf[0],
    surf[2],
    Rmax)) {
    RZWarning("Current radius is incompatible with conic offset.\n");
  } else {
    // Input plane: located at -f minus half the thickness

    m_inputBoundary->setRadius(m_radius);
    m_inputBoundary->setCurvatureRadius(surf[0].Rc);
    m_inputBoundary->setMedia(nullptr, &m_glass1);
    m_inputBoundary->setConicConstant(m_K[0]);
    m_inputBoundary->setConvex(surf[0].convex);
    m_inputBoundary->setCenterOffset(m_x0, m_y0);

    m_frontCap.setRadius(m_radius);
    m_frontCap.setCurvatureRadius(surf[0].Rc);
    m_frontCap.setConicConstant(m_K[0]);
    m_frontCap.setConvex(surf[0].convex);
    m_frontCap.setInvertNormals(false);
    m_frontCap.setCenterOffset(m_x0, m_y0);
    m_frontCap.requestRecalc();

    // Central plane: middle lens

    m_middleBoundary->setRadius(m_radius);
    m_middleBoundary->setCurvatureRadius(surf[1].Rc);
    m_middleBoundary->setMedia(&m_glass1, &m_glass2);
    m_middleBoundary->setConicConstant(m_K[1]);
    m_middleBoundary->setConvex(surf[1].convex);
    m_middleBoundary->setCenterOffset(m_x0, m_y0);
    
    m_middleCap.setRadius(m_radius);
    m_middleCap.setCurvatureRadius(surf[1].Rc);
    m_middleCap.setConicConstant(m_K[1]);
    m_middleCap.setConvex(surf[1].convex);
    m_middleCap.setInvertNormals(false);
    m_middleCap.setCenterOffset(m_x0, m_y0);
    m_middleCap.requestRecalc();

    // Output plane: opposite side

    m_outputBoundary->setRadius(m_radius);
    m_outputBoundary->setCurvatureRadius(surf[2].Rc);
    m_outputBoundary->setMedia(&m_glass2, nullptr);
    m_outputBoundary->setConicConstant(m_K[2]);
    m_outputBoundary->setConvex(surf[2].convex);
    m_outputBoundary->setCenterOffset(m_x0, m_y0);
  
    m_backCap.setRadius(m_radius);
    m_backCap.setCurvatureRadius(surf[2].Rc);
    m_backCap.setConicConstant(m_K[2]);
    m_backCap.setConvex(surf[2].convex);
    m_backCap.setInvertNormals(true);
    m_backCap.setCenterOffset(m_x0, m_y0);
    m_backCap.requestRecalc();
  
    m_cylinder.setHeight(m_edgeThickness1 + m_edgeThickness2);
    m_cylinder.setCaps(&m_frontCap, &m_backCap);

    // Intercept surfaces
    m_inputFrame->setDistance(frontPlaneZ * Vec3::eZ());
    m_inputFrame->recalculate();

    m_middleFrame->setDistance(middlePlaneZ * Vec3::eZ());
    m_middleFrame->recalculate();

    m_outputFrame->setDistance(backPlaneZ * Vec3::eZ());
    m_outputFrame->recalculate();
    
    setBoundingBox(
      Vec3(-m_radius + m_x0, -m_radius + m_y0, backPlaneZ + zSupVal),
      Vec3(m_radius + m_x0, m_radius + m_y0, frontPlaneZ + zInfVal)
    );
  
    refreshFrames();

  }
  
  updatePropertyValue("radius",   m_radius);
  updatePropertyValue("diameter", 2 * m_radius);
}

bool
ConicDoublet::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness1") {
    m_thickness1 = value;
    m_fromEdge1    = false;
  } else if (name == "edgeThickness1") {
    m_edgeThickness1 = value;
    m_fromEdge1    = true;
  } else if (name == "thickness2") {
    m_thickness2 = value;
    m_fromEdge2    = false;
  } else if (name == "edgeThickness2") {
    m_edgeThickness2 = value;
    m_fromEdge2    = true;
  } else if (name == "radius") {
    m_radius = value;
  } else if (name == "diameter") {
    m_radius = .5 * static_cast<Real>(value);
  } else if (name == "curvature") {
    return propertyChanged("frontCurvature", value)
        && propertyChanged("middleCurvature", value)
        && propertyChanged("backCurvature", value);
  } else if (name == "conic") {
    return propertyChanged("frontConic", value)
        && propertyChanged("middleConic", value)
        && propertyChanged("backConic", value);
  } else if (name == "frontCurvature") {
    m_rCurv[0]    = value;
  } else if (name == "frontConic") {
    m_K[0] = value;
  } else if (name == "middleCurvature") {
    m_rCurv[1]    = value;
  } else if (name == "middleConic") {
    m_K[1] = value;
  } else if (name == "backCurvature") {
    m_rCurv[2]    = value;
  } else if (name == "backConic") {
    m_K[2] = value;
  } else if (name == "x0") {
    m_x0 = value;
  } else if (name == "y0") {
    m_y0 = value;
  } else if (name == "n1") {
    m_glass1.n = value;
  } else if (name == "n2") {
    m_glass2.n = value;
  } else if (name == "vertexRelative") {
    m_vertexRelative = value;
  } else if (name == "referenceVertex") {
    int vtx = floor(static_cast<Real>(value));
    if (vtx < 0 || vtx > 2) {
      RZError("%s: surface index %d out of bounds\n", name.c_str(), vtx);
      return false;
    }

    m_referenceVtx = vtx;
  } else {
    return Element::propertyChanged(name, value);
  }

  recalcModel();

  return true;
}

ConicDoublet::ConicDoublet(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_glass1.type     = EMMediumIsotropic;
  m_glass1.n        = 1.5;
  m_glass2.type     = EMMediumIsotropic;
  m_glass2.n        = 1.5;
  
  m_inputBoundary  = new ConicLensBoundary;
  m_inputBoundary->setConvex(true);
  m_inputBoundary->setMedia(nullptr, &m_glass1);
  
  m_middleBoundary = new ConicLensBoundary;
  m_middleBoundary->setConvex(true); 
  m_middleBoundary->setMedia(&m_glass1, &m_glass2);
  
  m_outputBoundary = new ConicLensBoundary;
  m_outputBoundary->setConvex(false);
  m_outputBoundary->setMedia(&m_glass2, nullptr);

  m_inputFrame  = new TranslatedFrame("inputFrame",  frame, Vec3::zero());
  m_middleFrame = new TranslatedFrame("middleFrame", frame, Vec3::zero());
  m_outputFrame = new TranslatedFrame("outputFrame", frame, Vec3::zero());

  pushOpticalSurface("inputSurface",  m_inputFrame,  m_inputBoundary);
  pushOpticalSurface("middleSurface",  m_middleFrame,  m_middleBoundary);
  pushOpticalSurface("outputSurface", m_outputFrame, m_outputBoundary);

  m_objectPlane = new TranslatedFrame("objectPlane", frame, Vec3::zero());
  m_middlePlane = new TranslatedFrame("middlePlane", frame, Vec3::zero());
  m_imagePlane  = new TranslatedFrame("imagePlane",  frame, Vec3::zero());

  addPort("inputAperture",    m_inputFrame);
  addPort("middleAperture",   m_middleFrame);
  addPort("outputAperture",   m_outputFrame);
  
  addPort("objectPlane",      m_objectPlane);
  addPort("middlePlane",      m_middlePlane);
  addPort("imagePlane",       m_imagePlane);

  m_cylinder.setVisibleCaps(false, false);

  recalcModel();
}

ConicDoublet::~ConicDoublet()
{
  if (m_inputBoundary != nullptr)
    delete m_inputBoundary;

  if (m_middleBoundary != nullptr)
    delete m_middleBoundary;

  if (m_outputBoundary != nullptr)
    delete m_outputBoundary;
  
  if (m_objectPlane != nullptr)
    delete m_objectPlane;

  if (m_middlePlane != nullptr)
    delete m_middlePlane;
  
  if (m_imagePlane != nullptr)
    delete m_imagePlane;
}

void
ConicDoublet::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
ConicDoublet::renderOpenGL()
{
  // Full edge thickness of the multiplet
  Real fullThickness = m_edgeThickness1 + m_edgeThickness2;
  Real locZ;
  locZ = parentFrame()->toRelative(m_inputFrame->getCenter()).z;
  
  glTranslatef(0, 0,  locZ);
  material("input.lens");
  m_frontCap.display();

  glTranslatef(0, 0, -fullThickness);
  material("output.lens");
  m_backCap.display();
  
  material("lens");
  m_cylinder.display();
  
}
