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

#include <Elements/LensletArray.h>
#include <TranslatedFrame.h>

using namespace RZ;

RZ_DESCRIBE_OPTICAL_ELEMENT(LensletArray, "Rectangular grid of microlenses")
{
  property("conic",        0.0,  "Conic constant (K) of the lens surface");
  property("thickness",   1e-2,  "Thickness of the microlenses [m]");
  property("width",       1e-1,  "Horizontal dimension of the array [m]");
  property("height",      1e-1,  "Vertical dimension of the array [m]");
  property("cols",          10,  "Number of microlenses in the horizontal direction");
  property("rows",          10,  "Number of microlenses in the vertical direction");
  property("curvature",     1.,  "Radius of curvature of the microlenses [m]");
  property("focalLength", 5e-1,  "Focal length of the microlenses [m]");
  property("n",            1.5,  "Refractive index of the microlenses");
}

void
LensletArray::recalcModel()
{
  // Update processors
  m_inputProcessor->setWidth(m_width);
  m_inputProcessor->setHeight(m_height);
  m_inputProcessor->setCols(m_cols);
  m_inputProcessor->setRows(m_rows);
  m_inputProcessor->setCurvatureRadius(m_rCurv);
  m_inputProcessor->setRefractiveIndex(1, m_mu);
  m_inputProcessor->setConicConstant(m_K);

  m_outputProcessor->setWidth(m_width);
  m_outputProcessor->setHeight(m_height);
  m_outputProcessor->setCols(m_cols);
  m_outputProcessor->setRows(m_rows);
  m_outputProcessor->setCurvatureRadius(-m_rCurv);
  m_outputProcessor->setRefractiveIndex(m_mu, 1);
  m_outputProcessor->setConicConstant(m_K);
  
  // Get lenslet radius
  Real radius = m_inputProcessor->lensletRadius();

  m_inputFrame->setDistance(-.5 * m_thickness * Vec3::eZ());
  m_outputFrame->setDistance(+.5 * m_thickness * Vec3::eZ());

  m_cylinder.setHeight(m_thickness);
  m_cylinder.setRadius(radius);

  m_depth = m_rCurv - sqrt(m_rCurv * m_rCurv - radius * radius);
  m_f     = .5 * m_rCurv /  (m_mu - 1);

  m_inputFocalPlane->setDistance(-(.5 * m_thickness + m_f)* Vec3::eZ());
  m_outputFocalPlane->setDistance(+(.5 * m_thickness + m_f)* Vec3::eZ());
  
  m_objectPlane->setDistance(-(.5 * m_thickness + 2 * m_f)* Vec3::eZ());
  m_imagePlane->setDistance(+(.5 * m_thickness + 2 * m_f)* Vec3::eZ());

  m_cap.setRadius(radius);
  m_cap.setCurvatureRadius(-m_rCurv);
  m_cap.requestRecalc();
  
  m_cap.setInvertNormals(true);
  m_cylinder.setCaps(&m_cap, &m_cap);

  setBoundingBox(
      Vec3(-m_width / 2, -m_height / 2, -m_thickness / 2),
      Vec3(+m_width / 2, +m_height / 2, +m_thickness / 2));

  updatePropertyValue("focalLength",  m_f);
  updatePropertyValue("curvature",    m_rCurv);

  refreshFrames();
}

bool
LensletArray::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "conic") {
    m_K = value;
    recalcModel();
  } else if (name == "width") {
    m_width = value;
    recalcModel();
  } else if (name == "height") {
    m_height = value;
    recalcModel();
  } else if (name == "cols") {
    m_cols = value;
    recalcModel();
  } else if (name == "rows") {
    m_rows = value;
    recalcModel();
  } else if (name == "curvature") {
    m_rCurv = value;
    recalcModel();
  } else if (name == "focalLength") {
    m_rCurv = 2 * (Real) value * (m_mu - 1);
    recalcModel();
  } else if (name == "n") {
    m_mu = value;
    recalcModel();
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}

LensletArray::LensletArray(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_inputProcessor  = new LensletArrayProcessor;
  m_outputProcessor = new LensletArrayProcessor;

  m_inputProcessor->setConvex(true);
  m_outputProcessor->setConvex(false);

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

  m_cylinder.setVisibleCaps(true, true);
  
  refreshProperties();
}

LensletArray::~LensletArray()
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
LensletArray::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
LensletArray::renderOpenGL()
{
  unsigned int i, j;

  Real halfW  = .5 * m_width;
  Real halfH  = .5 * m_height;
  Real lensW  = m_width / m_cols;
  Real lensH  = m_height / m_rows;

  glTranslatef(0, 0, -.5 * m_thickness);

  for (j = 0; j < m_rows; ++j) {
    for (i = 0; i < m_cols; ++i) {
      glPushMatrix();
        glTranslatef(-halfW + (i + .5) * lensW, -halfH + (j + .5) * lensH, 0);

        material("lens");
        m_cylinder.display();

        glTranslatef(0, 0, .5 * m_thickness + m_depth);
        
        material("output.lens");
        //m_cap.display();
        
        glRotatef(180, 1, 0, 0);
        glTranslatef(0, 0, .5 * m_thickness + 2 * m_depth);
        
        material("input.lens");
        //m_cap.display();
      glPopMatrix();
    }
  }
}
