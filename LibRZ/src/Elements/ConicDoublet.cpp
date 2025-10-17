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

using namespace RZ;

RZ_DESCRIBE_OPTICAL_ELEMENT(ConicDoublet, "Lens with surfaces given by conic curves")
{
  property("thickness1",       1e-2, "Thickness of the first lens [m]");
  property("thickness2",       1e-2, "Thickness of the second lens [m]");
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
}

void
ConicDoublet::recalcModel()
{
  Real R2  = m_radius * m_radius;

  Real Rc[3], Rc2[3], sigma[3];
  Real dZ[3];
  
  Real n1 = m_glass1.n;
  Real n2 = m_glass2.n;

  bool convex[3];
  
  Real zSup[4];
  Real zInf[4];

  // Calculate properties of both surfaces.
  for (auto i = 0; i < 3; ++i) {
    double n    = (i < 2) ? n1 : n2;

    Rc[i]     = fabs(m_rCurv[i]);
    Rc2[i]    = m_rCurv[i]  * m_rCurv[i];
    convex[i] = m_rCurv[i] > 0;
    sigma[i]  = convex[i] ? 1 : -1;
    
    if (isZero(m_K[i] + 1))
      m_displacement[i] = .5 * R2 / m_rCurv[i];
    else
      m_displacement[i] = (Rc[i] - sqrt(Rc2[i] - (m_K[i] + 1) * R2)) / (m_K[i] + 1);
  }

  dZ[0] = dZ[1] = .5 * m_thickness1;
  dZ[2] = .5 * m_thickness2;
  
  const Real Rmax1 = RZ::ConicSurface::Rmax(
    sigma[0],
    Rc[0],
    m_K[0],
    m_displacement[0],
    sigma[1],
    Rc[1],
    m_K[1],
    m_displacement[1],
    m_thickness1);

  const Real Rmax2 = RZ::ConicSurface::Rmax(
    sigma[1],
    Rc[1],
    m_K[1],
    m_displacement[1],
    sigma[2],
    Rc[2],
    m_K[2],
    m_displacement[2],
    m_thickness2);
  
  const Real Rmax = std::min(Rmax1, Rmax2);
  const Real rho2 = m_x0 * m_x0 + m_y0 * m_y0;
  const Real rho  = sqrt(rho2);

  auto zVal = [&](int i, Real rad) { 
    Real r = rho - rad;
    Real r2 = r * r;
    if (m_K[i] == -1)
      return -sigma[i] * (.5 / Rc[i] * r2 - m_displacement[i]);
    else
      return -sigma[i] * ((Rc[i] - sqrt(Rc2[i] - (m_K[i] + 1) * r2)) / (m_K[i] + 1) - m_displacement[i]);
  };
  
  Real zSupVal;
  Real zInfVal;

  zSup[0] = (zVal(2, rho2));   // z(0,0) -> vertex
  zSup[1] = (zVal(2, 0));                           // z(x0, y0)
  zSup[2] = (zVal(2, +m_radius));                   // z(x0-r, y0-r)
  zSup[3] = (zVal(2, -m_radius));                   // z(x0+r, y0+r)
  zInf[0] = (zVal(0, rho2));   // z(0,0) -> vertex
  zInf[1] = (zVal(0, 0));                           // z(x0, y0)
  zInf[2] = (zVal(0, +m_radius));                   // z(x0-r, y0-r)
  zInf[3] = (zVal(0, -m_radius));                   // z(x0+r, y0+r)
  
  if (rho > (Rmax - m_radius)) {
    RZWarning("Current radius is incompatible with conic offset.\n");
  } else {
    if (rho2 < m_radius * m_radius) {
      zSupVal = fmin(zSup[0], fmin(zSup[1], fmin(zSup[2], zSup[3]))) - m_thickness2;
      zInfVal = fmax(zInf[0], fmax(zInf[1], fmax(zInf[2], zInf[3]))) + m_thickness1;
    } else {
      zSupVal = fmin(zSup[1], fmin(zSup[2], zSup[3])) - m_thickness2;
      zInfVal = fmax(zInf[1], fmax(zInf[2], zInf[3])) + m_thickness1;
    }
   
    // Input plane: located at -f minus half the thickness

    m_inputBoundary->setRadius(m_radius);
    m_inputBoundary->setCurvatureRadius(Rc[0]);
    m_inputBoundary->setMedia(nullptr, &m_glass1);
    m_inputBoundary->setConicConstant(m_K[0]);
    m_inputBoundary->setConvex(convex[0]);
    m_inputBoundary->setCenterOffset(m_x0, m_y0);

    m_frontCap.setRadius(m_radius);
    m_frontCap.setCurvatureRadius(Rc[0]);
    m_frontCap.setConicConstant(m_K[0]);
    m_frontCap.setConvex(convex[0]);
    m_frontCap.setInvertNormals(false);
    m_frontCap.setCenterOffset(m_x0, m_y0);
    m_frontCap.requestRecalc();

    // Central plane: middle lens

    m_middleBoundary->setRadius(m_radius);
    m_middleBoundary->setCurvatureRadius(Rc[1]);
    m_middleBoundary->setMedia(&m_glass1, &m_glass2);
    m_middleBoundary->setConicConstant(m_K[1]);
    m_middleBoundary->setConvex(convex[1]);
    m_middleBoundary->setCenterOffset(m_x0, m_y0);
    
    m_middleCap.setRadius(m_radius);
    m_middleCap.setCurvatureRadius(Rc[1]);
    m_middleCap.setConicConstant(m_K[1]);
    m_middleCap.setConvex(convex[1]);
    m_middleCap.setInvertNormals(false); // ???
    m_middleCap.setCenterOffset(m_x0, m_y0);
    m_middleCap.requestRecalc();

    // Output plane: opposite side

    m_outputBoundary->setRadius(m_radius);
    m_outputBoundary->setCurvatureRadius(Rc[2]);
    m_outputBoundary->setMedia(&m_glass2, nullptr);
    m_outputBoundary->setConicConstant(m_K[2]);
    m_outputBoundary->setConvex(convex[2]);
    m_outputBoundary->setCenterOffset(m_x0, m_y0);
  
    m_backCap.setRadius(m_radius);
    m_backCap.setCurvatureRadius(Rc[2]);
    m_backCap.setConicConstant(m_K[2]);
    m_backCap.setConvex(convex[2]);
    m_backCap.setInvertNormals(true);
    m_backCap.setCenterOffset(m_x0, m_y0);
    m_backCap.requestRecalc();
  
    m_cylinder.setHeight(m_thickness1 + m_thickness2);
    m_cylinder.setCaps(&m_frontCap, &m_backCap);

    // Intercept surfaces
    m_inputFrame->setDistance(+.5 * (m_thickness1 + m_thickness2) * Vec3::eZ());
    m_inputFrame->recalculate();

    m_middleFrame->setDistance(-.5 * (m_thickness1 - m_thickness2) * Vec3::eZ());
    m_middleFrame->recalculate();

    m_outputFrame->setDistance(-.5 * (m_thickness1 + m_thickness2) * Vec3::eZ());
    m_outputFrame->recalculate();

    setBoundingBox(
      Vec3(-m_radius + m_x0, -m_radius + m_y0, zSupVal), 
      Vec3(m_radius + m_x0, m_radius + m_y0, zInfVal)
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
  } else if (name == "thickness2") {
    m_thickness2 = value;
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
  m_middleBoundary->setConvex(true); // ??
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
  glTranslatef(0, 0,  -.5 * (m_thickness1 + m_thickness2));
  material("output.lens");
  m_backCap.display();
  
  material("lens");
  m_cylinder.display();

  glTranslatef(0, 0, (m_thickness1 + m_thickness2));
  material("input.lens");
  m_frontCap.display();
  
}

