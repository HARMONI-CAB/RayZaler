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

#include <Tripod.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <GL/glut.h>

using namespace RZ;

#define ROD_DEFAULT_LENGTH   5e-2
#define ROD_DEFAULT_DIAMETER 3e-3

bool
Tripod::propertyChanged(std::string const &name, PropertyValue const &val)
{
  Real value = val;
  
  if (name == "leg1") {
    m_leg1 = val;
    m_surface->setLeg(0, m_leg1);
    m_surface->recalculate();
  } else if (name == "leg2") {
    m_leg2 = val;
    m_surface->setLeg(1, m_leg2);
    m_surface->recalculate();
  } else if (name == "leg3") {
    m_leg3 = val;
    m_surface->setLeg(2, m_leg3);
    m_surface->recalculate();
  } else if (name == "radius") {
    m_ta_radius = val;
    m_surface->setRadius(m_ta_radius);
    m_surface->recalculate();
  } else if (name == "angle") {
    m_ta_angle  = val;
    m_surface->setAngle(m_ta_angle);
    m_surface->recalculate();
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

  m_glLegs[0].setRadius(.5 * m_legDiameter);
  m_glLegs[1].setRadius(.5 * m_legDiameter);
  m_glLegs[2].setRadius(.5 * m_legDiameter);

  m_glLegs[0].setHeight(m_leg1);
  m_glLegs[1].setHeight(m_leg2);
  m_glLegs[2].setHeight(m_leg3);
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
  registerProperty("leg1",   m_leg1);
  registerProperty("leg2",   m_leg2);
  registerProperty("leg3",   m_leg3);
  registerProperty("radius", m_ta_radius);
  registerProperty("angle",  m_ta_angle);

  registerProperty("legDiameter", m_legDiameter);

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

///////////////////////////////// Factory //////////////////////////////////////
std::string
TripodFactory::name() const
{
  return "Tripod";
}

Element *
TripodFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new Tripod(this, name, pFrame, parent);
}
