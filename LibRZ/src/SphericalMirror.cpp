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

#include <SphericalMirror.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
SphericalMirror::recalcModel()
{
  GLfloat rCurv = 2 * m_flength;
  GLfloat absR  = fabs(rCurv);
  
  // Displacement of the curvature center wrt the intercept surface
  m_displacement = sqrt(rCurv * rCurv - m_radius * m_radius);

  // If the curvature radius is negative, the center is on the other side
  if (rCurv < 0)
    m_displacement = -m_displacement;

  // Depth (or height) of the spherical surface w.r.t the edge. This is the
  // z distance of the center w.r.t the edge.
  m_depth = rCurv - m_displacement;

  m_topCap.setRadius(m_radius);
  m_topCap.setCurvatureRadius(rCurv);
  m_topCap.requestRecalc();
  m_topCap.setCenterOffset(m_x0, m_y0);

  m_bottomCap.setRadius(m_radius);
  m_bottomCap.setCurvatureRadius(rCurv);
  m_bottomCap.requestRecalc();
  m_bottomCap.setCenterOffset(m_x0, m_y0);
  
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setCaps(&m_topCap, &m_bottomCap);
  
  m_processor->setRadius(m_radius);
  m_processor->setFocalLength(m_flength);
  m_processor->setCenterOffset(m_x0, m_y0);

  m_reflectiveSurfaceFrame->setDistance(m_depth * Vec3::eZ());
  m_reflectiveSurfaceFrame->recalculate();
}

bool
SphericalMirror::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  Real maxDist = 2 * fabs(m_flength) - m_radius;
  Real maxR2   = maxDist * maxDist;

  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else if (name == "flength") {
    m_flength = value;
    recalcModel();
  } else if (name == "x0") {
    if ((Real) value * (Real) value + m_y0 * m_y0 > maxR2)
      return false;
    
    m_x0 = value;
    recalcModel();
  } else if (name == "y0") {
    if ((Real) value * (Real) value + m_x0 * m_x0 > maxR2)
      return false;
    
    m_y0 = value;
    recalcModel();
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}


SphericalMirror::SphericalMirror(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new SphericalMirrorProcessor;

  registerProperty("thickness",   1e-2);
  registerProperty("radius",    2.5e-2);
  registerProperty("flength",       1.);
  registerProperty("x0",            0.);
  registerProperty("y0",            0.);

  m_reflectiveSurfaceFrame = new TranslatedFrame("refSurf", frame, Vec3::zero());

  pushOpticalSurface("refSurf", m_reflectiveSurfaceFrame, m_processor);
  addPort("refPort", m_reflectiveSurfaceFrame);
  
  m_cylinder.setVisibleCaps(false, false);
  m_bottomCap.setInvertNormals(true);
  
  refreshProperties();
}

SphericalMirror::~SphericalMirror()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
SphericalMirror::nativeMaterialOpenGL(std::string const &)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
SphericalMirror::renderOpenGL()
{
  material("mirror");

  glTranslatef(0, 0, m_depth - m_thickness);

  material("input.mirror");

  m_topCap.display();
  m_cylinder.display();

  glTranslatef(0, 0, m_thickness);
  m_bottomCap.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
SphericalMirrorFactory::name() const
{
  return "SphericalMirror";
}

Element *
SphericalMirrorFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new SphericalMirror(this, name, pFrame, parent);
}
