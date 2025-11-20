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
#include <Logger.h>
#include <Surfaces/Conic.h>
#include <LensHelpers.h>
#include <SurfaceShape.h>

using namespace RZ;

RZ_DESCRIBE_OPTICAL_ELEMENT(ConicLens, "Lens with surfaces given by conic curves")
{
  property("thickness",         2e-2, "Thickness of the lens [m]");
  property("edgeThickness",     1e-2, "Thickness of side of the lens [m]");
  property("radius",          2.5e-2, "Radius of the lens [m]");
  property("diameter",    2 * 2.5e-2, "Diameter of the lens [m]");
  property("x0",                 0.0, "X-axis offset [m]");
  property("y0",                 0.0, "Y-axis offset [m]");
  property("n",                  1.5, "Refractive index");

  property("curvature",         1e-1, "Radius of curvature of both surfaces [m]");
  property("focalLength",       5e-2, "Focal length of both surfaces [m]");
  property("conic",              0.0, "Conic constant (K) of both surfaces");

  property("frontCurvature",    1e-1, "Radius of curvature of the front surface [m]");
  property("frontFocalLength",  5e-2, "Focal length of the front surface [m]");
  property("frontConic",         0.0, "Conic constant (K) of the front surface");

  property("backCurvature",     1e-1, "Radius of curvature of the front surface [m]");
  property("backFocalLength",   5e-2, "Focal length of the back surface [m]");
  property("backConic",          0.0, "Conic constant (K) of the back surface");
  
  property("vertexRelative",  false, "The element is placed with respect to a surface a vertex");
  property("referenceVertex",     0, "Surface index where the reference vertex is");

  property("apertureType", "elliptical", "Shape of the conic lens.");
  property("apertureHeight", 2.5e-2, "Height of the conic lens.");
  property("apertureWidth",  2.5e-2, "Width of the conic lens.");

}

GLAbstractCap *frontCap, *backCap;

void
ConicLens::recalcModel()
{

  Real n = m_glass.n;
  
  LensSurfaceProperties surf[2];
  
  Real frontPlaneZ, backPlaneZ;

  // Calculate properties of both surfaces.
  for (auto i = 0; i < 2; ++i) {
    if (m_fromFlen[i])
      m_rCurv[i]       = 2 * m_focalLength[i] * (n - 1);
    else
      m_focalLength[i] = .5 * m_rCurv[i] / (n - 1);
  }

  //surf[0].setProperties(+m_rCurv[0], m_K[0], m_radius, m_x0, m_y0);
  //surf[1].setProperties(-m_rCurv[1], m_K[1], m_radius, m_x0, m_y0);
  if (m_apertureType == Elliptical) {
    surf[0].setProperties(+m_rCurv[0], m_K[0], fmax(m_apertureHeight, m_apertureWidth), m_x0, m_y0);
    surf[1].setProperties(-m_rCurv[1], m_K[1], fmax(m_apertureHeight, m_apertureWidth), m_x0, m_y0);
  } else if (m_apertureType == Rectangular) {
    surf[0].setProperties(+m_rCurv[0], m_K[0], sqrt(m_apertureHeight * m_apertureHeight + m_apertureWidth * m_apertureWidth), m_x0, m_y0);
    surf[1].setProperties(-m_rCurv[1], m_K[1], sqrt(m_apertureHeight * m_apertureHeight + m_apertureWidth * m_apertureWidth), m_x0, m_y0);
  }

  if (!adjustThickness(
    m_edgeThickness,
    m_thickness,
    m_fromEdge,
    surf[0].sigma, 
    surf[0].displacement,
    surf[1].sigma, 
    surf[1].displacement))
     RZWarning("Invalid lens geometry: negative thickness or edge thickness.\n");
     
  if (m_vertexRelative) {           
    frontPlaneZ = -surf[0].sigma * surf[0].displacement;

    if (m_referenceVtx > 0)
      frontPlaneZ += m_thickness;

  } else {
    frontPlaneZ = .5 * m_edgeThickness;
  }
  backPlaneZ = frontPlaneZ - m_edgeThickness;
  
  const Real Rmax = LensSurfaceProperties::Rmax(surf[0], surf[1], m_edgeThickness);
 
  Real zSupVal = 0;
  Real zInfVal = 0;
  
  if (!LensSurfaceProperties::zLimits(zSupVal, zInfVal, surf[0], surf[1], Rmax)) {
    RZWarning("Current radius is incompatible with conic offset.\n");
  } else {
    // Input focal plane: located at -f minus half the thickness
    m_frontFocalPlane->setDistance(frontPlaneZ * Vec3::eZ());
    m_objectPlane->setDistance(frontPlaneZ * Vec3::eZ());

    m_inputBoundary->setRadius(m_radius);
    m_inputBoundary->setCurvatureRadius(surf[0].Rc);
    m_inputBoundary->setMedia(nullptr, &m_glass);
    m_inputBoundary->setConicConstant(m_K[0]);
    m_inputBoundary->setConvex(surf[0].convex);
    m_inputBoundary->setCenterOffset(m_x0, m_y0);
    m_inputBoundary->setApertureHeight(m_apertureHeight);
    m_inputBoundary->setApertureWidth(m_apertureWidth);
    m_inputBoundary->setApertureType(m_apertureType);

    // Output focal plane: opposite side
    m_backFocalPlane->setDistance(backPlaneZ * Vec3::eZ());
    m_imagePlane->setDistance(backPlaneZ * Vec3::eZ());

    m_outputBoundary->setRadius(m_radius);
    m_outputBoundary->setCurvatureRadius(surf[1].Rc);
    m_outputBoundary->setMedia(&m_glass, nullptr);
    m_outputBoundary->setConicConstant(m_K[1]);
    m_outputBoundary->setCenterOffset(m_x0, m_y0);
    m_outputBoundary->setConvex(surf[1].convex);
    m_outputBoundary->setApertureHeight(m_apertureHeight);
    m_outputBoundary->setApertureWidth(m_apertureWidth);
    m_outputBoundary->setApertureType(m_apertureType);

    if (m_apertureType == Elliptical) {
      m_frontEllipCap.setWidth(m_apertureWidth);
      m_frontEllipCap.setHeight(m_apertureHeight);
      m_frontEllipCap.setCurvatureRadius(surf[0].Rc);
      m_frontEllipCap.setConicConstant(m_K[0]);
      m_frontEllipCap.setConvex(surf[0].convex);
      m_frontEllipCap.setInvertNormals(false);
      m_frontEllipCap.setCenterOffset(m_x0, m_y0);
      m_frontEllipCap.requestRecalc();

      m_backEllipCap.setWidth(m_apertureWidth);
      m_backEllipCap.setHeight(m_apertureHeight);
      m_backEllipCap.setCurvatureRadius(surf[1].Rc);
      m_backEllipCap.setConicConstant(m_K[1]);
      m_backEllipCap.setConvex(surf[1].convex);
      m_backEllipCap.setInvertNormals(true);
      m_backEllipCap.setCenterOffset(m_x0, m_y0);
      m_backEllipCap.requestRecalc();
      frontCap = &m_frontEllipCap;
      backCap = &m_backEllipCap;
    } else if (m_apertureType == Rectangular) {
      m_frontRectCap.setWidth(m_apertureWidth);
      m_frontRectCap.setHeight(m_apertureHeight);
      m_frontRectCap.setCurvatureRadius(surf[0].Rc);
      m_frontRectCap.setConicConstant(m_K[0]);
      m_frontRectCap.setConvex(surf[0].convex);
      m_frontRectCap.setInvertNormals(false);
      m_frontRectCap.setCenterOffset(m_x0, m_y0);
      m_frontRectCap.requestRecalc();

      m_backRectCap.setWidth(m_apertureWidth);
      m_backRectCap.setHeight(m_apertureHeight);
      m_backRectCap.setCurvatureRadius(surf[1].Rc);
      m_backRectCap.setConicConstant(m_K[1]);
      m_backRectCap.setConvex(surf[1].convex);
      m_backRectCap.setInvertNormals(true);
      m_backRectCap.setCenterOffset(m_x0, m_y0);
      m_backRectCap.requestRecalc();
      frontCap = &m_frontRectCap;
      backCap = &m_backRectCap;
    }
  
    m_cylinder.setHeight(m_edgeThickness);
    m_cylinder.setCaps(frontCap, backCap); 
    
    // Intercept surfaces
    m_inputFrame->setDistance(frontPlaneZ * Vec3::eZ());
    m_inputFrame->recalculate();

    m_outputFrame->setDistance(backPlaneZ * Vec3::eZ());
    m_outputFrame->recalculate();
    
    setBoundingBox(
      //Vec3(-m_radius + m_x0, -m_radius + m_y0, backPlaneZ + zSupVal),
      //Vec3(m_radius + m_x0, m_radius + m_y0, frontPlaneZ + zInfVal)
      Vec3(-m_apertureWidth + m_x0, -m_apertureHeight + m_y0, backPlaneZ + zSupVal),
      Vec3(m_apertureWidth + m_x0, m_apertureHeight + m_y0, frontPlaneZ + zInfVal)
    );

    refreshFrames();

  } 
  
  updatePropertyValue("focalLength", 0.5 * (m_focalLength[0] + m_focalLength[1]));
  updatePropertyValue("curvature",   0.5 * (m_rCurv[0]       + m_rCurv[1]));

  updatePropertyValue("radius",   m_radius);
  updatePropertyValue("diameter", 2.0 * m_radius);
}

bool
ConicLens::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    m_fromEdge    = false;
  } else if (name == "edgeThickness") {
    m_edgeThickness = value;
    m_fromEdge    = true;
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
    m_glass.n = value;
  } else if (name == "vertexRelative") {
    m_vertexRelative = value;
  } else if (name == "referenceVertex") {
    int vtx = floor(static_cast<Real>(value));
    if (vtx < 0 || vtx > 1) {
      RZError("%s: surface index %d out of bounds\n", name.c_str(), vtx);
      return false;
    }
    m_referenceVtx = vtx;
  } else if (name == "apertureType") {
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
  m_glass.type     = EMMediumIsotropic;
  m_glass.n        = 1.5;

  m_inputBoundary  = new ConicLensBoundary;
  m_inputBoundary->setConvex(true);
  m_inputBoundary->setMedia(nullptr, &m_glass);

  m_outputBoundary = new ConicLensBoundary;
  m_outputBoundary->setConvex(false);
  m_outputBoundary->setMedia(&m_glass, nullptr);

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
  Real locZ;
  locZ = parentFrame()->toRelative(m_inputFrame->getCenter()).z;
  
  glTranslatef(0, 0, locZ);
  material("input.lens");
  frontCap->display();

  glTranslatef(0, 0, -m_edgeThickness);
  material("output.lens");
  backCap->display();
  
  material("lens");
  m_cylinder.display();
  
}
