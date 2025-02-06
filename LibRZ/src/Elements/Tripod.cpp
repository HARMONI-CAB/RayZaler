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

#include <Elements/Tripod.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <GL/glut.h>

using namespace RZ;

#define ROD_DEFAULT_LENGTH   5e-2
#define ROD_DEFAULT_DIAMETER 3e-3

RZ_DESCRIBE_ELEMENT(Tripod, "Flat mount supported by 3 legs with adjustable heights, forming an isosceles triangle")
{
  property("leg1",        2e-2,  "Height of the first leg");
  property("leg2",        2e-2,  "Height of the second leg");
  property("leg3",        2e-2,  "Height of the third leg");
  property("radius",      42e-3, "Radius of the circumscribing circle [m]");
  property("diameter",    84e-3, "Diameter of the circumscribing circle [m]");
  property("angle",       70,    "Angle of the vertex of the triangle [deg]");

  property("legDiameter", 6e-3, "Diameter of the legs [m]");
}

bool
Tripod::propertyChanged(std::string const &name, PropertyValue const &val)
{
  Real value = val;
  
  if (name == "leg1") {
    m_leg1 = val;
    m_surface->setLeg(0, m_leg1);
  } else if (name == "leg2") {
    m_leg2 = val;
    m_surface->setLeg(1, m_leg2);
  } else if (name == "leg3") {
    m_leg3 = val;
    m_surface->setLeg(2, m_leg3);
  } else if (name == "radius") {
    m_ta_radius = val;
    m_surface->setRadius(m_ta_radius);
  } else if (name == "diameter") {
    m_ta_radius = 0.5 * (Real) val;
    m_surface->setRadius(m_ta_radius);
  } else if (name == "angle") {
    m_ta_angle  = val;
    m_surface->setAngle(m_ta_angle);
  } else if (name == "legDiameter") {
    m_legDiameter  = val;
  } else {
    // Unrecognized property. Fallback to element property
    return Element::propertyChanged(name, val);
  }

  recalcLegs();

  return true;
}

void
Tripod::recalcLegs()
{
  Real legR = .5 * m_legDiameter;

  // Representation. 
  m_p[0] = Vec3(
      + m_ta_radius * sin(deg2rad(.5 * m_ta_angle)),
      + m_ta_radius * cos(deg2rad(.5 * m_ta_angle)),
        0);

  m_p[1] = Vec3(
      - m_ta_radius * sin(deg2rad(.5 * m_ta_angle)),
      + m_ta_radius * cos(deg2rad(.5 * m_ta_angle)),
        0);
  
  m_p[2] = Vec3(
      0,
      -m_ta_radius,
      0);

  m_glLegs[0].setRadius(legR);
  m_glLegs[1].setRadius(legR);
  m_glLegs[2].setRadius(legR);

  m_glLegs[0].setHeight(m_leg1);
  m_glLegs[1].setHeight(m_leg2);
  m_glLegs[2].setHeight(m_leg3);

  updatePropertyValue("radius",   m_ta_radius);
  updatePropertyValue("diameter", 2 * m_ta_radius);
  
  refreshFrames();

  Real maxH = fmax(m_leg1, fmax(m_leg2, m_leg3));

  setBoundingBox(
    Vec3(m_p[1].x - legR, m_p[2].y - legR, 0),
    Vec3(m_p[0].x + legR, m_p[1].y + legR, maxH));
}

void
Tripod::initTripod()
{
  int i;

  m_surface = new TripodFrame("tripodSurface", parentFrame());

  m_surface->setLeg(0, m_leg1);
  m_surface->setLeg(1, m_leg2);
  m_surface->setLeg(2, m_leg3);
  m_surface->setRadius(m_ta_radius);
  m_surface->setAngle(m_ta_angle);
  m_surface->recalculate();

  for (i = 0; i < 3; ++i) {
    m_glLegs[i].setSlices(24);
    m_glLegs[i].setVisibleCaps(true, true);
  }

  recalcLegs();
}

Tripod::Tripod(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent)
  : Element(factory, name, frame, parent)
{
  initTripod();
  addPort("surface", m_surface);
  refreshProperties();
}

void
Tripod::renderOpenGL()
{
  material("legs");

  for (int i = 0; i < 3; ++i) {
    glPushMatrix();
    glTranslatef(m_p[i].x, m_p[i].y, 0);
    m_glLegs[i].display();
    glPopMatrix();
  }
}

Tripod::~Tripod()
{
  if (m_surface != nullptr)
    delete m_surface;
}
