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

#include <Elements/ConicLens.h>
#include <TranslatedFrame.h>

using namespace RZ;

RZ_DESCRIBE_OPTICAL_ELEMENT(ConicLens, "Lens with surfaces given by conic curves")
{
  property("thickness",         1e-2,       "Thickness of the lens [m]");
  property("radius",            2.5e-2,     "Radius of the lens [m]");
  property("diameter",          2 * 2.5e-2, "Diameter of the lens [m]");
  property("x0",                0,          "X-axis offset [m]");
  property("y0",                0,          "Y-axis offset [m]");
  property("n",                 1.5,        "Refractive index");

  property("curvature",         1e-1,       "Radius of curvature of both surfaces [m]");
  property("focalLength",       5e-2,       "Focal length of both surfaces [m]");
  property("conic",             0,          "Conic constant (K) of both surfaces");

  property("frontCurvature",    1e-1,       "Radius of curvature of the front surface [m]");
  property("frontFocalLength",  5e-2,       "Focal length of the front surface [m]");
  property("frontConic",        0,          "Conic constant (K) of the front surface");

  property("backCurvature",     1e-1,       "Radius of curvature of the front surface [m]");
  property("backFocalLength",   5e-2,       "Focal length of the back surface [m]");
  property("backConic",         0,          "Conic constant (K) of the back surface");
}

void
ConicLens::recalcModel()
{
  Real R2  = m_radius * m_radius;

  Real Rc[2], Rc2[2], sigma[2];
  Real dZ[2];

  bool convex[2];

  // Calculate properties of both surfaces.
  for (auto i = 0; i < 2; ++i) {
    if (m_fromFlen[i])
      m_rCurv[i]       = 2 * m_focalLength[i] * (m_mu - 1);
    else
      m_focalLength[i] = .5 * m_rCurv[i] / (m_mu - 1);

    Rc[i]     = fabs(m_rCurv[i]);
    Rc2[i]    = m_rCurv[i]  * m_rCurv[i];
    convex[i] = m_rCurv[i] > 0;
    sigma[i]  = convex[i] ? 1 : -1;
    
    if (isZero(m_K[i] + 1))
      m_displacement[i] = .5 * R2 / m_rCurv[i];
    else
      m_displacement[i] = (Rc[i] - sqrt(Rc2[i] - (m_K[i] + 1) * R2)) / (m_K[i] + 1);
  }

#if  0
  auto R_1 = m_rCurv[0];
  auto R_2 = m_rCurv[1];
  auto dn  = (m_mu - 1) * m_thickness / m_mu;

  Real d    = m_thickness + m_displacement[0] + m_displacement[1];
  Real fInv = (m_mu - 1) * (1 / R_1 + 1 / R_2 + dn / (R_1 * R_2));
  Real FFD  = (1 + dn/ R_1) / fInv;
  Real BFD  = (1 + dn/ R_2) / fInv;

  printf("Effective F: %g\n", 1 / fInv);
  printf("Thickness: %g\n", m_thickness);
  printf("%g, %g\n", R_1, R_2);
  printf("FFD, BFD: %g, %g\n", FFD, BFD);
  printf("ffL, bfL: %g, %g\n", m_focalLength[0], m_focalLength[1]);

  dZ[0] = FFD - m_focalLength[0];
  dZ[1] = BFD - m_focalLength[1];
  #endif 

  dZ[0] = dZ[1] = .5 * m_thickness;
  
  // Input focal plane: located at -f minus half the thickness
  m_frontFocalPlane->setDistance(+(dZ[0] + m_focalLength[0])* Vec3::eZ());
  m_objectPlane->setDistance(+(dZ[0] + 2 * m_focalLength[0]) * Vec3::eZ());

  m_inputBoundary->setRadius(m_radius);
  m_inputBoundary->setCurvatureRadius(Rc[0]);
  m_inputBoundary->setRefractiveIndex(1, m_mu);
  m_inputBoundary->setConicConstant(m_K[0]);
  m_inputBoundary->setConvex(convex[0]);

  m_frontCap.setRadius(m_radius);
  m_frontCap.setCurvatureRadius(Rc[0]);
  m_frontCap.setConicConstant(m_K[0]);
  m_frontCap.setConvex(convex[0]);
  m_frontCap.setInvertNormals(false);
  m_frontCap.requestRecalc();

  // Output focal plane: opposite side
  m_backFocalPlane->setDistance(-(dZ[1] + m_focalLength[1]) * Vec3::eZ());
  m_imagePlane->setDistance(-(dZ[1] + 2 * m_focalLength[1]) * Vec3::eZ());

  m_outputBoundary->setRadius(m_radius);
  m_outputBoundary->setCurvatureRadius(Rc[1]);
  m_outputBoundary->setRefractiveIndex(m_mu, 1);
  m_outputBoundary->setConicConstant(m_K[1]);
  m_outputBoundary->setConvex(!convex[1]);
  
  m_backCap.setRadius(m_radius);
  m_backCap.setCurvatureRadius(Rc[1]);
  m_backCap.setConicConstant(m_K[1]);
  m_backCap.setConvex(!convex[1]);
  m_backCap.setInvertNormals(true);
  m_backCap.requestRecalc();
  
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setRadius(m_radius);

  // Intercept surfaces
  m_inputFrame->setDistance(+.5 * m_thickness * Vec3::eZ());
  m_inputFrame->recalculate();

  m_outputFrame->setDistance(-.5 * m_thickness * Vec3::eZ());
  m_outputFrame->recalculate();

  setBoundingBox(
      Vec3(-m_radius, -m_radius, fmin(-(.5 * m_thickness + m_displacement[1]), -m_thickness / 2)),
      Vec3(+m_radius, +m_radius, fmax(+(.5 * m_thickness + m_displacement[0]), +m_thickness / 2)));

  refreshFrames();

  updatePropertyValue("focalLength", 0.5 * (m_focalLength[0] + m_focalLength[1]));
  updatePropertyValue("curvature",   0.5 * (m_rCurv[0]       + m_rCurv[1]));

  updatePropertyValue("radius",   m_radius);
  updatePropertyValue("diameter", 2 * m_radius);
}

bool
ConicLens::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
  } else if (name == "radius") {
    m_radius = value;
  } else if (name == "diameter") {
    m_radius = .5 * static_cast<Real>(value);
  } else if (name == "focalLength") {
    return propertyChanged("frontFocalLength", value) 
        && propertyChanged("backFocalLength", value);
  } else if (name == "curvature") {
    return propertyChanged("frontCurvature", value)
        && propertyChanged("backCurvature", value);
  } else if (name == "conic") {
    return propertyChanged("frontConic", value)
        && propertyChanged("backConic", value);
  } else if (name == "frontFocalLength") {
    m_focalLength[0] = static_cast<Real>(value);
    m_fromFlen[0]    = true;
  } else if (name == "frontCurvature") {
    m_rCurv[0]    = value;
    m_fromFlen[0] = false;
  } else if (name == "frontConic") {
    m_K[0] = value;
  } else if (name == "backFocalLength") {
    m_focalLength[1] = static_cast<Real>(value);
    m_fromFlen[1]    = true;
  } else if (name == "backCurvature") {
    m_rCurv[1]    = value;
    m_fromFlen[1] = false;
  } else if (name == "backConic") {
    m_K[1] = value;
  } else if (name == "x0") {
    m_x0 = value;
  } else if (name == "y0") {
    m_y0 = value;
  } else if (name == "n") {
    m_mu = value;
  } else {
    return Element::propertyChanged(name, value);
  }

  recalcModel();

  return true;
}

ConicLens::ConicLens(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_inputBoundary  = new ConicLensBoundary;
  m_outputBoundary = new ConicLensBoundary;

  m_inputBoundary->setConvex(true);
  m_outputBoundary->setConvex(false);

  m_inputFrame  = new TranslatedFrame("inputFrame",  frame, Vec3::zero());
  m_outputFrame = new TranslatedFrame("outputFrame", frame, Vec3::zero());

  pushOpticalSurface("inputSurface",  m_inputFrame,  m_inputBoundary);
  pushOpticalSurface("outputSurface", m_outputFrame, m_outputBoundary);

  // Create helper planes. These are exposed as ports
  m_frontFocalPlane  = new TranslatedFrame("frontFocalPlane", frame, Vec3::zero());
  m_backFocalPlane   = new TranslatedFrame("backFocalPlane", frame, Vec3::zero());

  m_objectPlane      = new TranslatedFrame("objectPlane", frame, Vec3::zero());
  m_imagePlane       = new TranslatedFrame("imagePlane", frame, Vec3::zero());

  addPort("inputAperture",    m_inputFrame);
  addPort("outputAperture",   m_outputFrame);
  
  addPort("frontFocalPlane",  m_frontFocalPlane);
  addPort("backFocalPlane",   m_backFocalPlane);
  addPort("objectPlane",      m_objectPlane);
  addPort("imagePlane",       m_imagePlane);

  m_cylinder.setVisibleCaps(false, false);

  recalcModel();
}

ConicLens::~ConicLens()
{
  if (m_inputBoundary != nullptr)
    delete m_inputBoundary;

  if (m_outputBoundary != nullptr)
    delete m_outputBoundary;

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
ConicLens::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
ConicLens::renderOpenGL()
{
  glTranslatef(0, 0,  -.5 * m_thickness);
  material("output.lens");
  m_backCap.display();
  
  material("lens");
  m_cylinder.display();

  glTranslatef(0, 0, m_thickness);
  material("input.lens");
  m_frontCap.display();
  
}
