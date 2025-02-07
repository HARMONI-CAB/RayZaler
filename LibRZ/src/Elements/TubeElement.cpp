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

#include <Elements/TubeElement.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <GL/glut.h>

using namespace RZ;

#define ROD_DEFAULT_LENGTH   5e-2
#define ROD_DEFAULT_DIAMETER 3e-3

RZ_DESCRIBE_ELEMENT(TubeElement, "A hollow tube with circular section and open ends")
{
  property("length",        5e-2,   "Length of the tube [m]");
  property("innerDiameter", 1.5e-3, "Inner diameter [m]");
  property("outerDiameter", 3e-3,   "Outer diameter [m]");
}

void
TubeElement::recalcBoundingBox()
{
  m_sides[0]->setDistance(m_cachedLength * Vec3::eZ());
  m_sides[1]->setDistance(
        0.5 * m_cachedOuterDiameter * Vec3::eZ()
      + 0.5 * m_cachedLength   * Vec3::eX());
  
  setBoundingBox(
    Vec3(-m_cachedOuterDiameter / 2, -m_cachedOuterDiameter / 2, 0),
    Vec3(+m_cachedOuterDiameter / 2, +m_cachedOuterDiameter / 2, m_cachedLength));
}

bool
TubeElement::propertyChanged(std::string const &name, PropertyValue const &val)
{
  Real value = val;
  bool middleChanged = false;

  if (name == "length") {
    m_sides[0]->setDistance(value * Vec3::eZ());
    m_sides[0]->recalculate();
    m_cachedLength = value;
    middleChanged = true;
  } else if (name == "outerDiameter") {
    m_cachedOuterDiameter = value;
    middleChanged = true;
  } else if (name == "innerDiameter") {
    m_cachedInnerDiameter = value;
  } else {
    return Element::propertyChanged(name, val);
  }
  
  if (middleChanged) {
    m_sides[1]->setDistance(0.5 * m_cachedLength   * Vec3::eX());
    m_sides[1]->recalculate();
  }

  m_tube.setHeight(m_cachedLength);
  m_tube.setOuterRadius(.5 * m_cachedOuterDiameter);
  m_tube.setInnerRadius(.5 * m_cachedInnerDiameter);

  recalcBoundingBox();

  return true;
}

void
TubeElement::initSides()
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
      deg2rad(rotations[i][0]));
  
  m_sides[0] = new TranslatedFrame(
      "top",
      m_rotatedSides[0],
      m_cachedLength * Vec3::eZ());

  m_sides[1] = new TranslatedFrame(
      "middle",
      m_rotatedSides[1],
      0.5 * m_cachedOuterDiameter * Vec3::eZ() + 0.5 * m_cachedLength * Vec3::eX());

  m_sides[2] = new TranslatedFrame(
      "bottom",
      m_rotatedSides[2], 0 * Vec3::eZ());

  m_tube.setHeight(m_cachedLength);
  m_tube.setOuterRadius(.5 * m_cachedOuterDiameter);
  m_tube.setInnerRadius(.5 * m_cachedInnerDiameter);
  m_tube.setVisibleCaps(true, true);
  m_tube.setSlices(24);
}

TubeElement::TubeElement(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent)
  : Element(factory, name, frame, parent)
{
  initSides();

  addPort("top",    m_sides[0]);
  addPort("middle", m_sides[1]);
  addPort("bottom", m_sides[2]);

  refreshProperties();
}

void
TubeElement::renderOpenGL()
{
  material("rod");

  m_tube.display();
}


TubeElement::~TubeElement()
{
  for (int i = 0; i < 3; ++i) {
    delete m_sides[i];
    delete m_rotatedSides[i];
  }
}
