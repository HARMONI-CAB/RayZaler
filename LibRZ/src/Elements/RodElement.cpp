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

#include <Elements/RodElement.h>

#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <GL/glut.h>

using namespace RZ;

#define ROD_DEFAULT_LENGTH   5e-2
#define ROD_DEFAULT_DIAMETER 3e-3

RZ_DESCRIBE_ELEMENT(RodElement, "Solid bar with circular section")
{
  property("length",   ROD_DEFAULT_LENGTH,         "Length of the rod [m]");
  property("diameter", ROD_DEFAULT_DIAMETER,       "Diameter of the section [m]");
  property("radius",   0.5 * ROD_DEFAULT_DIAMETER, "Radius of the section [m]");
}

void
RodElement::recalcBoundingBox()
{
  m_sides[0]->setDistance(m_cachedLength * Vec3::eZ());
  m_sides[1]->setDistance(
        0.5 * m_cachedDiameter * Vec3::eX()
      + 0.5 * m_cachedLength   * Vec3::eZ());
  
  setBoundingBox(
    Vec3(-m_cachedDiameter / 2, -m_cachedDiameter / 2, -m_cachedLength/2),
    Vec3(+m_cachedDiameter / 2, +m_cachedDiameter / 2, +m_cachedLength/2));

  updatePropertyValue("diameter", m_cachedDiameter);
  updatePropertyValue("radius", 0.5 * m_cachedDiameter);

  refreshFrames();
}

bool
RodElement::propertyChanged(std::string const &name, PropertyValue const &val)
{
  if (name == "length")
    m_cachedLength = val;
  else if (name == "radius")
    m_cachedDiameter = 2. * (Real) val;
  else if (name == "diameter")
    m_cachedDiameter = val;
  else
    return Element::propertyChanged(name, val);

  recalcBoundingBox();

  return true;
}

void
RodElement::initSides()
{
  const char *names[] = {
    "top", "middle", "bottom"
  };

  Real rotations[][4] = {
    {  0, 0, 1, 0}, // Top
    {-90, 0, 1, 0}, // Middle
    {180, 0, 1, 0}  // Bottom
  };

  for (int i = 0; i < 3; ++i)
    m_rotatedSides[i] = new RotatedFrame(
      std::string(names[i]) + "_rotation",
      parentFrame(),
      Vec3(rotations[i][1], rotations[i][2], rotations[i][3]),
      rotations[i][0]);
  
  m_sides[0] = new TranslatedFrame(
      "top",
      m_rotatedSides[0],
      m_cachedLength * Vec3::eZ());

  m_sides[1] = new TranslatedFrame(
      "middle",
      m_rotatedSides[1],
        0.5 * m_cachedDiameter * Vec3::eX()
      + 0.5 * m_cachedLength   * Vec3::eZ());

  m_sides[2] = new TranslatedFrame(
      "bottom",
      m_rotatedSides[2], 0 * Vec3::eZ());

  m_cylinder.setHeight(m_cachedLength);
  m_cylinder.setRadius(.5 * m_cachedDiameter);
  m_cylinder.setVisibleCaps(true, true);
  m_cylinder.setSlices(24);

  recalcBoundingBox();
}

RodElement::RodElement(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent)
  : Element(factory, name, frame, parent)
{
  m_cachedLength   = ROD_DEFAULT_LENGTH;
  m_cachedDiameter = ROD_DEFAULT_DIAMETER;

  initSides();

  addPort("top",    m_sides[0]);
  addPort("middle", m_sides[1]);
  addPort("bottom", m_sides[2]);

  refreshProperties();
}

void
RodElement::renderOpenGL()
{
  material("rod");

  m_cylinder.display();
}


RodElement::~RodElement()
{
  for (int i = 0; i < 3; ++i) {
    delete m_sides[i];
    delete m_rotatedSides[i];
  }
}
