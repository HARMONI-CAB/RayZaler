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

#include <ConvexLens.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
ConvexLens::recalcModel()
{
  // This is the thickness of the concave part
  m_depth = m_rCurv - sqrt(m_rCurv * m_rCurv - m_radius * m_radius);

  // Focal length
  if (m_fromFlen)
    m_rCurv = 2 * m_f * (m_mu - 1);
  else
    m_f     = .5 * m_rCurv /  (m_mu - 1);

  // Input focal plane: located at -f minus half the thickness
  m_inputFocalPlane->setDistance(-(.5 * m_thickness + m_f)* Vec3::eZ());

  // Output focal plane: opposite side
  m_outputFocalPlane->setDistance(+(.5 * m_thickness + m_f)* Vec3::eZ());
  
  m_objectPlane->setDistance(-(.5 * m_thickness + 2 * m_f)* Vec3::eZ());
  m_imagePlane->setDistance(+(.5 * m_thickness + 2 * m_f)* Vec3::eZ());

  m_topCap.setRadius(m_radius);
  m_topCap.setCurvatureRadius(m_rCurv);
  m_topCap.requestRecalc();

  m_bottomCap.setRadius(m_radius);
  m_bottomCap.setCurvatureRadius(-m_rCurv);
  m_bottomCap.requestRecalc();
  
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setCaps(&m_topCap, &m_bottomCap);

  m_inputProcessor->setRadius(m_radius);
  m_inputProcessor->setCurvatureRadius(m_rCurv);
  m_inputProcessor->setRefractiveIndex(1, m_mu);

  m_outputProcessor->setRadius(m_radius);
  m_outputProcessor->setCurvatureRadius(-m_rCurv);
  m_outputProcessor->setRefractiveIndex(m_mu, 1);
  
  // Intercept surfaces
  m_inputFrame->setDistance(-.5 * m_thickness * Vec3::eZ());
  m_outputFrame->setDistance(+.5 * m_thickness * Vec3::eZ());
}

bool
ConvexLens::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else if (name == "curvature") {
    m_rCurv = value;
    m_fromFlen = false;
    recalcModel();
  } else if (name == "fLen") {
    m_f = value;
    m_fromFlen = true;
    recalcModel();
  } else if (name == "n") {
    m_mu = value;
    recalcModel();
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}

ConvexLens::ConvexLens(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_inputProcessor  = new SphericalLensProcessor;
  m_outputProcessor = new SphericalLensProcessor;

  m_inputProcessor->setConvex(true);
  m_outputProcessor->setConvex(false);

  registerProperty("thickness",   1e-2);
  registerProperty("radius",    2.5e-2);
  registerProperty("curvature",     1.);
  registerProperty("n",            1.5);
  registerProperty("fLen",          1.);

  m_inputFrame  = new TranslatedFrame("inputSurf",  frame, Vec3::zero());
  m_outputFrame = new TranslatedFrame("outputSurf", frame, Vec3::zero());

  pushOpticalSurface("inputFace",  m_inputFrame,  m_inputProcessor);
  pushOpticalSurface("outputFace", m_outputFrame, m_outputProcessor);

  // Create helper planes. These are exposed as ports
  m_inputFocalPlane  = new TranslatedFrame("inputFocalPlane", frame, Vec3::zero());
  m_outputFocalPlane = new TranslatedFrame("outputFocalPlane", frame, Vec3::zero());

  m_objectPlane      = new TranslatedFrame("objectPlane", frame, Vec3::zero());
  m_imagePlane       = new TranslatedFrame("imagePlane", frame, Vec3::zero());

  addPort("inputFocalPlane",  m_inputFocalPlane);
  addPort("outputFocalPlane", m_outputFocalPlane);
  addPort("objectPlane",      m_objectPlane);
  addPort("imagePlane",       m_imagePlane);

  m_cylinder.setVisibleCaps(false, false);
  m_bottomCap.setInvertNormals(true);

  refreshProperties();
}

ConvexLens::~ConvexLens()
{
  if (m_inputProcessor != nullptr)
    delete m_inputProcessor;

  if (m_outputProcessor != nullptr)
    delete m_outputProcessor;

  if (m_inputFocalPlane != nullptr)
    delete m_inputFocalPlane;
  
  if (m_outputFocalPlane != nullptr)
    delete m_outputFocalPlane;
  
  if (m_objectPlane != nullptr)
    delete m_objectPlane;
  
  if (m_imagePlane != nullptr)
    delete m_imagePlane;
}

void
ConvexLens::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
ConvexLens::renderOpenGL()
{
  material("mirror");

  glTranslatef(0, 0,  -.5 * m_thickness);

  material("input.mirror");

  m_topCap.display();
  m_cylinder.display();

  glTranslatef(0, 0, m_thickness);
  material("output.mirror");
  m_bottomCap.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
ConvexLensFactory::name() const
{
  return "ConvexLens";
}

Element *
ConvexLensFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new ConvexLens(this, name, pFrame, parent);
}
