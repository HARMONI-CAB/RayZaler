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

#include <Elements/ConicTriplet.h>
#include <TranslatedFrame.h>

using namespace RZ;

RZ_DESCRIBE_OPTICAL_ELEMENT(ConicTriplet, "Lens with surfaces given by conic curves")
{
  property("thickness1",         1e-2,       "Thickness of the first lens [m]");
  property("thickness2",         1e-2,       "Thickness of the second lens [m]");
  property("thickness3",         1e-2,       "Thickness of the second lens [m]");
  property("radius",            2.5e-2,      "Radius of the conic lense triplet [m]");
  property("radius1",            2.5e-2,     "Radius of the first lens [m]");
  property("radius2",            2.5e-2,     "Radius of the second lens [m]");
  property("radius3",            2.5e-2,     "Radius of the second lens [m]");
  property("diameter",          2 * 2.5e-2,  "Diameter of the conic lense triplet [m]");
  property("diameter1",          2 * 2.5e-2, "Diameter of the first lens [m]");
  property("diameter2",          2 * 2.5e-2, "Diameter of the second lens [m]");
  property("diameter3",          2 * 2.5e-2, "Diameter of the second lens [m]"); // check if radius and offset must be the same for both lenses or can be different
  property("x0",                 0.0,        "X-axis offset [m]");
  property("y0",                 0.0,        "Y-axis offset [m]");
  property("n1",                  1.5,        "Refractive index");
  property("n2",                  1.5,        "Refractive index");
  property("n3",                  1.5,        "Refractive index");

  property("curvature",          1e-1,       "Radius of curvature of the three surfaces [m]");
  property("focalLength",        5e-2,       "Focal length of the three surfaces [m]");
  property("conic",              0.0,        "Conic constant (K) of the three surfaces");

  property("frontCurvature",     1e-1,       "Radius of curvature of the front surface [m]");
  property("frontFocalLength",   5e-2,       "Focal length of the front surface [m]");
  property("frontConic",         0.0,        "Conic constant (K) of the front surface");

  property("middleCurvature1",      1e-1,       "Radius of curvature of the first middle surface [m]");
  property("middleFocalLength1",    5e-2,       "Focal length of the first middle surface [m]");
  property("middleConic1",          0.0,        "Conic constant (K) of the first middle surface");
  
  property("middleCurvature2",      1e-1,       "Radius of curvature of the second middle surface [m]");
  property("middleFocalLength2",    5e-2,       "Focal length of the second middle surface [m]");
  property("middleConic2",          0.0,        "Conic constant (K) of the second middle surface");

  property("backCurvature",      1e-1,       "Radius of curvature of the back surface [m]");
  property("backFocalLength",    5e-2,       "Focal length of the back surface [m]");
  property("backConic",          0.0,        "Conic constant (K) of the back surface");
}

void
ConicTriplet::recalcModel()
{
  Real R2_1  = m_radius1 * m_radius1;
  Real R2_2  = m_radius2 * m_radius2;
  Real R2_3  = m_radius3 * m_radius3;

  Real Rc[4], Rc2[4], sigma[4];
  Real dZ[4];

  bool convex[4];

  // Calculate properties of the four surfaces.
  for (auto i = 0; i < 4; ++i) {

    //double m_mu    = (i < 2) ? m_mu1 : m_mu2;
    //double R2 = (i < 2) ? R2_1 : R2_2;
    
    double m_mu;
    if (i == 0)
      m_mu = m_mu1;
    else if (i == 1)
      m_mu = m_mu2;
    else 
      m_mu = m_mu3;
    double R2;
    if (i == 0)
      R2 = R2_1;
    else if (i == 1)
      R2= R2_2;
    else 
      R2 = R2_3;

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

  dZ[0] = dZ[1] = .5 * m_thickness1;
  dZ[2] = dZ[3] = .5 * m_thickness2; // ??

  // Input focal plane: located at -f minus half the thickness
  m_frontFocalPlane->setDistance(+(dZ[0] + m_focalLength[0])* Vec3::eZ());
  m_objectPlane->setDistance(+(dZ[0] + 2 * m_focalLength[0]) * Vec3::eZ());

  m_inputBoundary->setRadius(m_radius1);
  m_inputBoundary->setCurvatureRadius(Rc[0]);
  m_inputBoundary->setRefractiveIndex(1, m_mu1);
  m_inputBoundary->setConicConstant(m_K[0]);
  m_inputBoundary->setConvex(convex[0]);

  m_frontCap.setRadius(m_radius1);
  m_frontCap.setCurvatureRadius(Rc[0]);
  m_frontCap.setConicConstant(m_K[0]);
  m_frontCap.setConvex(convex[0]);
  m_frontCap.setInvertNormals(false);
  m_frontCap.requestRecalc();

  // Second focal plane: first middle lens
  m_frontFocalPlane->setDistance(+(dZ[1] + m_focalLength[1])* Vec3::eZ());
  m_objectPlane->setDistance(+(dZ[1] + 2 * m_focalLength[1]) * Vec3::eZ());

  m_middleBoundary1->setRadius(fmin(m_radius1, m_radius2));
  m_middleBoundary1->setCurvatureRadius(Rc[1]);
  m_middleBoundary1->setRefractiveIndex(m_mu1, m_mu2);
  m_middleBoundary1->setConicConstant(m_K[1]);
  m_middleBoundary1->setConvex(convex[1]);
  
  m_middleCap1.setRadius(fmin(m_radius1, m_radius2));
  m_middleCap1.setCurvatureRadius(Rc[1]);
  m_middleCap1.setConicConstant(m_K[1]);
  m_middleCap1.setConvex(convex[1]);
  m_middleCap1.setInvertNormals(false); // ???
  m_middleCap1.requestRecalc();
  
  // Third focal plane: second middle lens
  m_frontFocalPlane->setDistance(+(dZ[2] + m_focalLength[2])* Vec3::eZ());
  m_objectPlane->setDistance(+(dZ[2] + 2 * m_focalLength[2]) * Vec3::eZ());

  m_middleBoundary2->setRadius(fmin(m_radius2, m_radius3));
  m_middleBoundary2->setCurvatureRadius(Rc[2]);
  m_middleBoundary2->setRefractiveIndex(m_mu2, m_mu3);
  m_middleBoundary2->setConicConstant(m_K[2]);
  m_middleBoundary2->setConvex(convex[2]);
  
  m_middleCap2.setRadius(fmin(m_radius2, m_radius3));
  m_middleCap2.setCurvatureRadius(Rc[2]);
  m_middleCap2.setConicConstant(m_K[2]);
  m_middleCap2.setConvex(convex[2]);
  m_middleCap2.setInvertNormals(false); // ???
  m_middleCap2.requestRecalc();


  // Output focal plane: opposite side
  m_backFocalPlane->setDistance(-(dZ[3] + m_focalLength[3]) * Vec3::eZ());
  m_imagePlane->setDistance(-(dZ[3] + 2 * m_focalLength[3]) * Vec3::eZ());

  m_outputBoundary->setRadius(m_radius2);
  m_outputBoundary->setCurvatureRadius(Rc[3]);
  m_outputBoundary->setRefractiveIndex(m_mu3, 1);
  m_outputBoundary->setConicConstant(m_K[3]);
  m_outputBoundary->setConvex(!convex[3]);
  
  m_backCap.setRadius(m_radius2);
  m_backCap.setCurvatureRadius(Rc[3]);
  m_backCap.setConicConstant(m_K[3]);
  m_backCap.setConvex(!convex[3]);
  m_backCap.setInvertNormals(true);
  m_backCap.requestRecalc();
  
  
  m_cylinder.setHeight(m_thickness1 + m_thickness2 + m_thickness3);
  m_cylinder.setRadius(std::min({m_radius1, m_radius2, m_radius3}));

  // Intercept surfaces
  //m_inputFrame->setDistance(+.5 * (m_thickness1 + m_thickness2 + m_thickness3) * Vec3::eZ());
  m_inputFrame->setDistance(.5 * (m_thickness1 + m_thickness2 + m_thickness3) * Vec3::eZ());
  m_inputFrame->recalculate();

  m_middleFrame1->setDistance(-.5 * (m_thickness1 - m_thickness2 - m_thickness3) * Vec3::eZ()); // ???
  m_middleFrame1->recalculate();
  
  m_middleFrame2->setDistance(-.5 * (m_thickness1 + m_thickness2 - m_thickness3) * Vec3::eZ()); // ???
  m_middleFrame2->recalculate();

  //m_outputFrame->setDistance(-.5 * (m_thickness1 + m_thickness2 + m_thickness3) * Vec3::eZ());
  m_outputFrame->setDistance(-.5 * (m_thickness1 + m_thickness2 + m_thickness3) * Vec3::eZ());
  m_outputFrame->recalculate();

  setBoundingBox(
      Vec3(-std::max({m_radius1, m_radius2, m_radius3}), -std::max({m_radius1, m_radius2, m_radius3}), fmin(-(m_thickness1 + m_thickness2 + m_thickness3 + m_displacement[3]) / 4, -(m_thickness1 + m_thickness2 + m_thickness3) / 3)),
      Vec3(+std::max({m_radius1, m_radius2, m_radius3}), +std::max({m_radius1, m_radius2, m_radius3}), fmax(+(m_thickness1 + m_thickness2 + m_thickness3 + m_displacement[0]) / 4, +(m_thickness1 + m_thickness2 + m_thickness3) / 3)));

  refreshFrames();

  updatePropertyValue("focalLength", (m_focalLength[0] + m_focalLength[1] + m_focalLength[2] + m_focalLength[3]) / 4);
  updatePropertyValue("curvature",   (m_rCurv[0] + m_rCurv[1] + m_rCurv[2] + m_rCurv[3]) / 4);

  updatePropertyValue("radius",   std::max({m_radius1, m_radius2, m_radius2}));
  updatePropertyValue("diameter", 2 * std::max({m_radius1, m_radius2, m_radius2}));
}

bool
ConicTriplet::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness1") {
    m_thickness1 = value;
  } else if (name == "thickness2") {
    m_thickness2 = value;
  } else if (name == "thickness3") {
    m_thickness3 = value;
  } else if (name == "radius1") {
    m_radius1 = value;
  } else if (name == "radius2") {
    m_radius2 = value;
  } else if (name == "radius3") {
    m_radius3 = value;
  } else if (name == "diameter1") {
    m_radius1 = .5 * static_cast<Real>(value);
  } else if (name == "diameter2") {
    m_radius2 = .5 * static_cast<Real>(value);
  } else if (name == "diameter3") {
    m_radius3 = .5 * static_cast<Real>(value);
  } else if (name == "focalLength") {
    return propertyChanged("frontFocalLength", value) 
        && propertyChanged("middleFocalLength1", value)
        && propertyChanged("middleFocalLength2", value)
        && propertyChanged("backFocalLength", value);
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
  } else if (name == "frontFocalLength") {
    m_focalLength[0] = static_cast<Real>(value);
    m_fromFlen[0]    = true;
  } else if (name == "frontCurvature") {
    m_rCurv[0]    = value;
    m_fromFlen[0] = false;
  } else if (name == "frontConic") {
    m_K[0] = value;
  } else if (name == "middleFocalLength1") {
    m_focalLength[1] = static_cast<Real>(value);
    m_fromFlen[1]    = true;
  } else if (name == "middleCurvature1") {
    m_rCurv[1]    = value;
    m_fromFlen[1] = false;
  } else if (name == "middleConic1") {
    m_K[1] = value;
  } else if (name == "middleFocalLength2") {
    m_focalLength[2] = static_cast<Real>(value);
    m_fromFlen[2]    = true;
  } else if (name == "middleCurvature2") {
    m_rCurv[2]    = value;
    m_fromFlen[2] = false;
  } else if (name == "middleConic2") {
    m_K[2] = value;
  } else if (name == "backFocalLength") {
    m_focalLength[3] = static_cast<Real>(value);
    m_fromFlen[3]    = true;
  } else if (name == "backCurvature") {
    m_rCurv[3]    = value;
    m_fromFlen[3] = false;
  } else if (name == "backConic") {
    m_K[3] = value;
  } else if (name == "x0") {
    m_x0 = value;
  } else if (name == "y0") {
    m_y0 = value;
  } else if (name == "n1") {
    m_mu1 = value;
  } else if (name == "n2") {
    m_mu2 = value;
  } else if (name == "n3") {
    m_mu3 = value;
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
  m_inputBoundary  = new ConicTripletBoundary;
  m_middleBoundary1 = new ConicTripletBoundary;
  m_middleBoundary2 = new ConicTripletBoundary;
  m_outputBoundary = new ConicTripletBoundary;

  m_inputBoundary->setConvex(true);
  m_middleBoundary1->setConvex(false); // ??
  m_middleBoundary2->setConvex(false); // ??
  m_outputBoundary->setConvex(false);

  m_inputFrame  = new TranslatedFrame("inputFrame",  frame, Vec3::zero());
  m_middleFrame1  = new TranslatedFrame("middleFrame1",  frame, Vec3::zero());
  m_middleFrame2  = new TranslatedFrame("middleFrame2",  frame, Vec3::zero());
  m_outputFrame = new TranslatedFrame("outputFrame", frame, Vec3::zero());

  pushOpticalSurface("inputSurface",  m_inputFrame,  m_inputBoundary);
  pushOpticalSurface("middleSurface1",  m_middleFrame1,  m_middleBoundary1);
  pushOpticalSurface("middleSurface2",  m_middleFrame2,  m_middleBoundary2);
  pushOpticalSurface("outputSurface", m_outputFrame, m_outputBoundary);

  // Create helper planes. These are exposed as ports
  m_frontFocalPlane  = new TranslatedFrame("frontFocalPlane", frame, Vec3::zero());
  m_middleFocalPlane1   = new TranslatedFrame("middleFocalPlane1", frame, Vec3::zero());
  m_middleFocalPlane2   = new TranslatedFrame("middleFocalPlane2", frame, Vec3::zero());
  m_backFocalPlane   = new TranslatedFrame("backFocalPlane", frame, Vec3::zero());

  m_objectPlane      = new TranslatedFrame("objectPlane", frame, Vec3::zero());
  m_middlePlane1       = new TranslatedFrame("middlePlane1", frame, Vec3::zero());
  m_middlePlane2       = new TranslatedFrame("middlePlane2", frame, Vec3::zero());
  m_imagePlane       = new TranslatedFrame("imagePlane", frame, Vec3::zero());

  addPort("inputAperture",    m_inputFrame);
  addPort("middleAperture1",   m_middleFrame1);
  addPort("middleAperture2",   m_middleFrame2);
  addPort("outputAperture",   m_outputFrame);
  
  addPort("frontFocalPlane",  m_frontFocalPlane);
  addPort("middleFocalPlane1", m_middleFocalPlane1);
  addPort("middleFocalPlane2", m_middleFocalPlane2);
  addPort("backFocalPlane",   m_backFocalPlane);
  addPort("objectPlane",      m_objectPlane);
  addPort("middlePlane1",       m_middlePlane1);
  addPort("middlePlane2",       m_middlePlane2);
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

  if (m_frontFocalPlane != nullptr)
    delete m_frontFocalPlane;

  if (m_middleFocalPlane1 != nullptr)
    delete m_middleFocalPlane1;
    
  if (m_middleFocalPlane2 != nullptr)
    delete m_middleFocalPlane2;
  
  if (m_backFocalPlane != nullptr)
    delete m_backFocalPlane;
  
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
  glTranslatef(0, 0,  -.5 * (m_thickness1 + m_thickness2 + m_thickness3));
  material("output.lens");
  m_backCap.display();
  
  material("lens");
  m_cylinder.display();

  glTranslatef(0, 0, m_thickness3);
  material("input.lens");
  m_frontCap.display();
  
}

