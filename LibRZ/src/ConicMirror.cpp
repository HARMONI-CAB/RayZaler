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

#include <ConicMirror.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
ConicMirror::recalcModel()
{
  Real R2  = m_radius * m_radius;
  Real Rc  = fabs(m_rCurv);
  Real Rc2 = m_rCurv  * m_rCurv;
  bool convex = m_rCurv < 0;
  Real sigma = convex ? 1 : -1;

  if (isZero(m_K + 1)) {
    m_displacement = .5 * R2 / m_rCurv;
  } else {
    m_displacement = (Rc - sqrt(Rc2 - (m_K + 1) * R2)) / (m_K + 1);
  }

  m_cap.setRadius(m_radius);
  m_cap.setCurvatureRadius(Rc);
  m_cap.setConicConstant(m_K);
  m_cap.setConvex(convex);
  m_cap.setInvertNormals(false);
  m_cap.setCenterOffset(m_x0, m_y0);
  
  m_rearCap.setRadius(m_radius);
  m_rearCap.setCurvatureRadius(Rc);
  m_rearCap.setConicConstant(m_K);
  m_rearCap.setConvex(convex);
  m_rearCap.setInvertNormals(true);
  m_rearCap.setCenterOffset(m_x0, m_y0);

  m_cylinder.setHeight(m_thickness);
  m_cylinder.setCaps(&m_cap, &m_rearCap);

  m_processor->setRadius(m_radius);
  m_processor->setCurvatureRadius(Rc);
  m_processor->setConicConstant(m_K);
  m_processor->setConvex(convex);
  m_processor->setCenterOffset(m_x0, m_y0);

  m_reflectiveSurfaceFrame->setDistance((m_thickness - sigma * m_displacement) * Vec3::eZ());
  m_reflectiveSurfacePort->setDistance((m_thickness - sigma * m_displacement) * Vec3::eZ());

  m_reflectiveSurfaceFrame->recalculate();
  m_reflectiveSurfacePort->recalculate();
}

bool
ConicMirror::propertyChanged(
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
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}

ConicMirror::ConicMirror(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new ConicMirrorProcessor;

  registerProperty("thickness",   1e-2);
  registerProperty("radius",    2.5e-2);
  registerProperty("curvature",  10e-2);
  registerProperty("conic",          0);
  registerProperty("x0",            0.);
  registerProperty("y0",            0.);
  
  m_reflectiveSurfaceFrame = new TranslatedFrame("refSurf", frame, Vec3::zero());
  m_reflectiveSurfacePort  = new TranslatedFrame("refPort", frame, Vec3::zero());

  pushOpticalSurface("refSurf", m_reflectiveSurfaceFrame, m_processor);
  addPort("refPort", m_reflectiveSurfacePort);

  m_cylinder.setVisibleCaps(false, false);
  m_cap.setInvertNormals(true);
  m_rearCap.setInvertNormals(false);
  
  refreshProperties();
}

ConicMirror::~ConicMirror()
{
  if (m_processor != nullptr)
    delete m_processor;
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

  material("mirror");

  glTranslatef(0, 0, -sigma * m_displacement);

  m_rearCap.display();
  m_cylinder.display();

  material("input.mirror");

  glTranslatef(0, 0, m_thickness);
  m_cap.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
ConicMirrorFactory::name() const
{
  return "ConicMirror";
}

Element *
ConicMirrorFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new ConicMirror(this, name, pFrame, parent);
}
