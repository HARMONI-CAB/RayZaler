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

#include <ConicLens.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
ConicLens::recalcModel()
{
  Real R2  = m_radius * m_radius;

  if (m_fromFlen)
    m_rCurv = 2 * m_focalLength * (m_mu - 1);
  else
    m_focalLength = .5 * m_rCurv / (m_mu - 1);

  Real Rc  = fabs(m_rCurv);
  Real Rc2 = m_rCurv  * m_rCurv;
  bool convex = m_rCurv > 0;
  Real sigma = convex ? 1 : -1;
  
  if (isZero(m_K + 1))
    m_displacement = .5 * R2 / m_rCurv;
  else
    m_displacement = (Rc - sqrt(Rc2 - (m_K + 1) * R2)) / (m_K + 1);
  
  // Input focal plane: located at -f minus half the thickness
  m_frontFocalPlane->setDistance(-(.5 * m_thickness + m_focalLength)* Vec3::eZ());

  // Output focal plane: opposite side
  m_backFocalPlane->setDistance(+(.5 * m_thickness + m_focalLength) * Vec3::eZ());
  
  m_objectPlane->setDistance(-(.5 * m_thickness + 2 * m_focalLength) * Vec3::eZ());
  m_imagePlane->setDistance(+(.5 * m_thickness + 2 * m_focalLength) * Vec3::eZ());

  m_topCap.setRadius(m_radius);
  m_topCap.setCurvatureRadius(Rc);
  m_topCap.setConicConstant(m_K);
  m_topCap.setConvex(!convex);
  m_topCap.setInvertNormals(true);
  m_topCap.requestRecalc();

  m_bottomCap.setRadius(m_radius);
  m_bottomCap.setCurvatureRadius(Rc);
  m_bottomCap.setConicConstant(m_K);
  m_bottomCap.setConvex(convex);
  m_bottomCap.setInvertNormals(false);
  m_bottomCap.requestRecalc();
  
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setCaps(&m_topCap, &m_bottomCap);

  m_inputProcessor->setRadius(m_radius);
  m_inputProcessor->setCurvatureRadius(Rc);
  m_inputProcessor->setRefractiveIndex(1, m_mu);
  m_inputProcessor->setConicConstant(m_K);
  m_inputProcessor->setConvex(!convex);

  m_outputProcessor->setRadius(m_radius);
  m_outputProcessor->setCurvatureRadius(Rc);
  m_outputProcessor->setRefractiveIndex(m_mu, 1);
  m_outputProcessor->setConicConstant(m_K);
  m_outputProcessor->setConvex(convex);
  
  // Intercept surfaces
  m_inputFrame->setDistance(-.5 * m_thickness * Vec3::eZ());
  m_outputFrame->setDistance(+.5 * m_thickness * Vec3::eZ());

  setBoundingBox(
      Vec3(-m_radius, -m_radius, -m_thickness / 2),
      Vec3(+m_radius, +m_radius, +m_thickness / 2));
}

bool
ConicLens::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else if (name == "diameter") {
    m_radius = .5 * static_cast<Real>(value);
    recalcModel();
  } else if (name == "focalLength") {
    m_focalLength = static_cast<Real>(value);
    m_fromFlen = true;
    recalcModel();
  } else if (name == "curvature") {
    m_rCurv = value;
    m_fromFlen = false;
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
  } else if (name == "n") {
    m_mu = value;
    recalcModel();
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}

ConicLens::ConicLens(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_inputProcessor  = new ConicLensProcessor;
  m_outputProcessor = new ConicLensProcessor;

  m_inputProcessor->setConvex(true);
  m_outputProcessor->setConvex(false);

  registerProperty("thickness",    1e-2);
  registerProperty("radius",     2.5e-2);
  registerProperty("diameter",     5e-2);
  registerProperty("curvature",   10e-2);
  registerProperty("focalLength",  5e-2);
  registerProperty("conic",          0.);
  registerProperty("x0",             0.);
  registerProperty("y0",             0.);
  registerProperty("n",            1.5);

  m_inputFrame  = new TranslatedFrame("inputFrame",  frame, Vec3::zero());
  m_outputFrame = new TranslatedFrame("outputFrame", frame, Vec3::zero());

  pushOpticalSurface("inputSurface",  m_inputFrame,  m_inputProcessor);
  pushOpticalSurface("outputSurface", m_outputFrame, m_outputProcessor);

  // Create helper planes. These are exposed as ports
  m_frontFocalPlane  = new TranslatedFrame("frontFocalPlane", frame, Vec3::zero());
  m_backFocalPlane   = new TranslatedFrame("backFocalPlane", frame, Vec3::zero());

  m_objectPlane      = new TranslatedFrame("objectPlane", frame, Vec3::zero());
  m_imagePlane       = new TranslatedFrame("imagePlane", frame, Vec3::zero());

  addPort("frontFocalPlane",  m_frontFocalPlane);
  addPort("backFocalPlane",   m_backFocalPlane);
  addPort("objectPlane",      m_objectPlane);
  addPort("imagePlane",       m_imagePlane);

  m_cylinder.setVisibleCaps(false, false);
  m_bottomCap.setInvertNormals(true);

  refreshProperties();
}

ConicLens::~ConicLens()
{
  if (m_inputProcessor != nullptr)
    delete m_inputProcessor;

  if (m_outputProcessor != nullptr)
    delete m_outputProcessor;

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
  material("lens");

  glTranslatef(0, 0,  -.5 * m_thickness);

  material("input.lens");

  m_topCap.display();
  m_cylinder.display();

  glTranslatef(0, 0, m_thickness);
  material("output.lens");
  m_bottomCap.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
ConicLensFactory::name() const
{
  return "ConicLens";
}

Element *
ConicLensFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new ConicLens(this, name, pFrame, parent);
}
