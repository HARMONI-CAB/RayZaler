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

#include <Elements/ConicTriplet.h>
#include <TranslatedFrame.h>
#include <Logger.h>
#include <Surfaces/Conic.h>
#include <LensHelpers.h>

using namespace RZ;

RZ_DESCRIBE_OPTICAL_ELEMENT(ConicTriplet, "Lens with surfaces given by conic curves")
{
  property("thickness1",        3e-2, "Thickness of the first lens [m]");
  property("edgeThickness1",    1e-2, "Thickness of the edge of the first lens [m]");
  property("thickness2",        3e-2, "Thickness of the second lens [m]");
  property("edgeThickness2",    1e-2, "Thickness of the edge of the second lens [m]");
  property("thickness3",        3e-2, "Thickness of the second lens [m]");
  property("edgeThickness3",    1e-2, "Thickness of the edge of the third lens [m]");
  property("radius",            5e-2, "Radius of the conic lens triplet [m]");
  property("diameter",          5e-2, "Diameter of the conic lense triplet [m]");
  property("x0",                0.0, "X-axis offset [m]");
  property("y0",                 0.0, "Y-axis offset [m]");
  property("n1",                 1.5, "Refractive index");
  property("n2",                 1.5, "Refractive index");
  property("n3",                 1.5, "Refractive index");

  property("conic",              0.0, "Conic constant (K) of the three surfaces");

  property("frontCurvature",    1e-1, "Radius of curvature of the front surface [m]");
  property("frontConic",         0.0, "Conic constant (K) of the front surface");

  property("middleCurvature1",  1e-1, "Radius of curvature of the first middle surface [m]");
  property("middleConic1",       0.0, "Conic constant (K) of the first middle surface");
  
  property("middleCurvature2",  1e-1, "Radius of curvature of the second middle surface [m]");
  property("middleConic2",       0.0, "Conic constant (K) of the second middle surface");

  property("backCurvature",     1e-1, "Radius of curvature of the back surface [m]");
  property("backConic",          0.0, "Conic constant (K) of the back surface");
}

void
ConicTriplet::recalcModel()
{
  Real dZ[4];
  
  LensSurfaceProperties surf[4];
  
  // Calculate properties of the four surfaces.
  for (auto i = 0; i < 4; ++i)
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
  
  if (!adjustThickness(
    m_edgeThickness3,
    m_thickness3,
    m_fromEdge3,
    surf[2].sigma, 
    surf[2].displacement,
    surf[3].sigma, 
    surf[3].displacement))
     RZWarning("Invalid lens geometry: negative thickness or edge thickness in the third lens.\n");
  
  dZ[0] = dZ[1] = .5 * m_edgeThickness1;
  dZ[2] = dZ[3] = .5 * m_edgeThickness2; 
  
  const Real Rmax1 = LensSurfaceProperties::Rmax(surf[0], surf[1], m_edgeThickness1);
  const Real Rmax2 = LensSurfaceProperties::Rmax(surf[1], surf[2], m_edgeThickness2);
  const Real Rmax3 = LensSurfaceProperties::Rmax(surf[2], surf[3], m_edgeThickness3);
  
  const Real Rmax  = std::min(Rmax1, std::min(Rmax2, Rmax3));
  
  Real zSupVal = - (.5 * m_edgeThickness2 + m_edgeThickness3);
  Real zInfVal = + (.5 * m_edgeThickness2 + m_edgeThickness1);
  
  if (!LensSurfaceProperties::zLimits(
    zSupVal,
    zInfVal,
    surf[0],
    surf[3],
    Rmax)) {
    RZWarning("Current radius is incompatible with conic offset.\n");
  } else {
    // Input surface: located at -f minus half the thickness

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

    // Second surface: first middle lens

    m_middleBoundary1->setRadius(m_radius);
    m_middleBoundary1->setCurvatureRadius(surf[1].Rc);
    m_middleBoundary1->setMedia(&m_glass1, &m_glass2);
    m_middleBoundary1->setConicConstant(m_K[1]);
    m_middleBoundary1->setConvex(surf[1].convex);
    m_middleBoundary1->setCenterOffset(m_x0, m_y0);
  
    m_middleCap1.setRadius(m_radius);
    m_middleCap1.setCurvatureRadius(surf[1].Rc);
    m_middleCap1.setConicConstant(m_K[1]);
    m_middleCap1.setConvex(surf[1].convex);
    m_middleCap1.setInvertNormals(false); 
    m_middleCap1.setCenterOffset(m_x0, m_y0);
    m_middleCap1.requestRecalc();
  
    // Third surface plane: second middle lens

    m_middleBoundary2->setRadius(m_radius);
    m_middleBoundary2->setCurvatureRadius(surf[2].Rc);
    m_middleBoundary2->setMedia(&m_glass2, &m_glass3);
    m_middleBoundary2->setConicConstant(m_K[2]);
    m_middleBoundary2->setConvex(surf[2].convex);
    m_middleBoundary2->setCenterOffset(m_x0, m_y0);
  
    m_middleCap2.setRadius(m_radius);
    m_middleCap2.setCurvatureRadius(surf[2].Rc);
    m_middleCap2.setConicConstant(m_K[2]);
    m_middleCap2.setConvex(surf[2].convex);
    m_middleCap2.setInvertNormals(false); 
    m_middleCap2.setCenterOffset(m_x0, m_y0);
    m_middleCap2.requestRecalc();


    // Output surface plane: opposite side

    m_outputBoundary->setRadius(m_radius);
    m_outputBoundary->setCurvatureRadius(surf[3].Rc);
    m_outputBoundary->setMedia(&m_glass3, nullptr);
    m_outputBoundary->setConicConstant(m_K[3]);
    m_outputBoundary->setConvex(surf[3].convex);
    m_outputBoundary->setCenterOffset(m_x0, m_y0);
  
    m_backCap.setRadius(m_radius);
    m_backCap.setCurvatureRadius(surf[3].Rc);
    m_backCap.setConicConstant(m_K[3]);
    m_backCap.setConvex(surf[3].convex);
    m_backCap.setInvertNormals(true);
    m_backCap.setCenterOffset(m_x0, m_y0);
    m_backCap.requestRecalc();
  
  
    m_cylinder.setHeight(m_edgeThickness1 + m_edgeThickness2 + m_edgeThickness3);
    m_cylinder.setCaps(&m_frontCap, &m_backCap);

    // Intercept surfaces
    m_inputFrame->setDistance(.5 * (m_edgeThickness1 + m_edgeThickness2 + m_edgeThickness3) * Vec3::eZ());
    m_inputFrame->recalculate();

    m_middleFrame1->setDistance(-.5 * (m_edgeThickness1 - m_edgeThickness2 - m_edgeThickness3) * Vec3::eZ()); 
    m_middleFrame1->recalculate();
  
    m_middleFrame2->setDistance(-.5 * (m_edgeThickness1 + m_edgeThickness2 - m_edgeThickness3) * Vec3::eZ()); 
    m_middleFrame2->recalculate();

    m_outputFrame->setDistance(-.5 * (m_edgeThickness1 + m_edgeThickness2 + m_edgeThickness3) * Vec3::eZ());
    m_outputFrame->recalculate();

    setBoundingBox(
      Vec3(-m_radius + m_x0, -m_radius + m_y0, zSupVal - .5 * (m_edgeThickness1 - m_edgeThickness3)), 
      Vec3(m_radius + m_x0, m_radius + m_y0, zInfVal - .5 * (m_edgeThickness1 - m_edgeThickness3))
    );

    refreshFrames();

  }

  updatePropertyValue("radius",   m_radius);
  updatePropertyValue("diameter", 2 * m_radius);
}

bool
ConicTriplet::propertyChanged(
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
  } else if (name == "thickness3") {
    m_thickness3 = value;
    m_fromEdge3    = false;
  } else if (name == "edgeThickness3") {
    m_edgeThickness3 = value;
    m_fromEdge3    = true;
  } else if (name == "radius") {
    m_radius = value;
  } else if (name == "diameter") {
    m_radius = .5 * static_cast<Real>(value);
  } else if (name == "curvature") {
    return propertyChanged("frontCurvature", value)
        && propertyChanged("middleCurvature1", value)
        && propertyChanged("middleCurvature2", value)
        && propertyChanged("backCurvature", value);
  } else if (name == "conic") {
    return propertyChanged("frontConic", value)
        && propertyChanged("middleConic1", value)
        && propertyChanged("middleConic2", value)
        && propertyChanged("backConic", value);
  } else if (name == "frontCurvature") {
    m_rCurv[0]    = value;
  } else if (name == "frontConic") {
    m_K[0] = value;
  } else if (name == "middleCurvature1") {
    m_rCurv[1]    = value;
  } else if (name == "middleConic1") {
    m_K[1] = value;
  } else if (name == "middleCurvature2") {
    m_rCurv[2]    = value;
  } else if (name == "middleConic2") {
    m_K[2] = value;
  } else if (name == "backCurvature") {
    m_rCurv[3]    = value;
  } else if (name == "backConic") {
    m_K[3] = value;
  } else if (name == "x0") {
    m_x0 = value;
  } else if (name == "y0") {
    m_y0 = value;
  } else if (name == "n1") {
    m_glass1.n = value;
  } else if (name == "n2") {
    m_glass2.n = value;
  } else if (name == "n3") {
    m_glass3.n = value;
  } else {
    return Element::propertyChanged(name, value);
  }

  recalcModel();

  return true;
}

ConicTriplet::ConicTriplet(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_inputBoundary   = new ConicLensBoundary;
  m_middleBoundary1 = new ConicLensBoundary;
  m_middleBoundary2 = new ConicLensBoundary;
  m_outputBoundary  = new ConicLensBoundary;

  m_inputBoundary->setConvex(true);
  m_middleBoundary1->setConvex(false); 
  m_middleBoundary2->setConvex(false);
  m_outputBoundary->setConvex(false);

  m_inputFrame    = new TranslatedFrame("inputFrame",    frame, Vec3::zero());
  m_middleFrame1  = new TranslatedFrame("middleFrame1",  frame, Vec3::zero());
  m_middleFrame2  = new TranslatedFrame("middleFrame2",  frame, Vec3::zero());
  m_outputFrame   = new TranslatedFrame("outputFrame",   frame, Vec3::zero());

  pushOpticalSurface("inputSurface",   m_inputFrame,   m_inputBoundary);
  pushOpticalSurface("middleSurface1", m_middleFrame1, m_middleBoundary1);
  pushOpticalSurface("middleSurface2", m_middleFrame2, m_middleBoundary2);
  pushOpticalSurface("outputSurface",  m_outputFrame,  m_outputBoundary);

  // Create helper planes. These are exposed as ports

  m_objectPlane   = new TranslatedFrame("objectPlane",  frame, Vec3::zero());
  m_middlePlane1  = new TranslatedFrame("middlePlane1", frame, Vec3::zero());
  m_middlePlane2  = new TranslatedFrame("middlePlane2", frame, Vec3::zero());
  m_imagePlane    = new TranslatedFrame("imagePlane",   frame, Vec3::zero());

  addPort("inputAperture",    m_inputFrame);
  addPort("middleAperture1",  m_middleFrame1);
  addPort("middleAperture2",  m_middleFrame2);
  addPort("outputAperture",   m_outputFrame);
  
  addPort("objectPlane",      m_objectPlane);
  addPort("middlePlane1",     m_middlePlane1);
  addPort("middlePlane2",     m_middlePlane2);
  addPort("imagePlane",       m_imagePlane);

  m_cylinder.setVisibleCaps(false, false);

  recalcModel();
}

ConicTriplet::~ConicTriplet()
{
  if (m_inputBoundary != nullptr)
    delete m_inputBoundary;

  if (m_middleBoundary1 != nullptr)
    delete m_middleBoundary1;
    
  if (m_middleBoundary2 != nullptr)
    delete m_middleBoundary2;

  if (m_outputBoundary != nullptr)
    delete m_outputBoundary;
  
  if (m_objectPlane != nullptr)
    delete m_objectPlane;

  if (m_middlePlane1 != nullptr)
    delete m_middlePlane1;
    
  if (m_middlePlane2 != nullptr)
    delete m_middlePlane2;
  
  if (m_imagePlane != nullptr)
    delete m_imagePlane;
}

void
ConicTriplet::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
ConicTriplet::renderOpenGL()
{
  glTranslatef(0, 0,  -.5 * (m_edgeThickness1 + m_edgeThickness2 + m_edgeThickness3));
  material("output.lens");
  m_backCap.display();
  
  material("lens");
  m_cylinder.display();

  glTranslatef(0, 0, (m_edgeThickness1 + m_edgeThickness2 + m_edgeThickness3));
  material("input.lens");
  m_frontCap.display();
  
}
