//
//  Copyright (c) 2025 Gonzalo José Carracedo Carballal
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

#include <Elements/WedgeElement.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <GL/glut.h>

using namespace RZ;

#define WEDGE_DEFAULT_LENGTH  1.
#define WEDGE_DEFAULT_WIDTH   1.
#define WEDGE_DEFAULT_HEIGHT1 .5
#define WEDGE_DEFAULT_HEIGHT2 1.

RZ_DESCRIBE_ELEMENT(WedgeElement, "Rectangular prism with adjustable dimensions")
{
  property("length",    WEDGE_DEFAULT_LENGTH,   "Length (X dimension) of the wedge [m]");
  property("width",     WEDGE_DEFAULT_WIDTH,    "Width (Y dimension) of the wedge [m]");
  property("height1",   WEDGE_DEFAULT_HEIGHT1,  "Height (Z dimension) of the wedge at negative X [m]");
  property("height2",   WEDGE_DEFAULT_HEIGHT2,  "Height (Z dimension) of the wedge at positive X [m]");
}

void
WedgeElement::recalculateTopSide()
{
  auto meanHeight = .5 * (m_cachedHeights[0] + m_cachedHeights[1]);
  
  m_sides[4]->setDistance(0);
  m_sides[4]->recalculate();
  m_sides[5]->setDistance(meanHeight * Vec3::eZ());
  m_sides[5]->recalculate();

  // Calculate rotation angle. This can be done via arctan2
  auto dh = m_cachedHeights[0] - m_cachedHeights[1];
  auto dx = m_cachedLength;
  auto angle = atan2(dh, dx);
  m_rotatedSides[4]->setAngle(angle);
  m_rotatedSides[4]->recalculate();
}

bool
WedgeElement::propertyChanged(std::string const &name, PropertyValue const &val)
{
  Real value = val;
  bool hadIt = false;

  if (name == "length") {
    m_sides[0]->setDistance(.5 * value * Vec3::eZ());
    m_sides[1]->setDistance(.5 * value * Vec3::eZ());
    m_sides[0]->recalculate();
    m_sides[1]->recalculate();
    m_cachedLength = value;
    hadIt = true;
  } else if (name == "width") {
    m_sides[2]->setDistance(.5 * value * Vec3::eZ());
    m_sides[3]->setDistance(.5 * value * Vec3::eZ());
    m_sides[2]->recalculate();
    m_sides[3]->recalculate();
    m_cachedWidth = value;
    hadIt = true;
  } else if (name == "height1") {
    m_cachedHeights[0] = value;
    recalculateTopSide();
    hadIt = true;
  } else if (name == "height2") {
    m_cachedHeights[1] = value;
    recalculateTopSide();
    hadIt = true;
  }
  
  if (hadIt) {
    auto meanHeight = .5 * (m_cachedHeights[0] + m_cachedHeights[1]);
    auto maxHeight  = fmax(m_cachedHeights[0], m_cachedHeights[1]);

    setBoundingBox(
      Vec3(-m_cachedLength / 2, -m_cachedWidth/2, -meanHeight),
      Vec3(+m_cachedLength / 2, +m_cachedWidth/2, -meanHeight + maxHeight));

    m_wedge.setLength(m_cachedLength);
    m_wedge.setWidth(m_cachedWidth);
    m_wedge.setHeights(m_cachedHeights[0], m_cachedHeights[1]);

    return true;
  }
  
  
  return Element::propertyChanged(name, val);
}

void
WedgeElement::initSides()
{
  const char *names[] = {
    "front", "back",
    "right", "left",
    "top",   "bottom"
  };

  Real rotations[][4] = {
    {+90, 0, 1, 0},
    {-90, 0, 1, 0},
    {-90, 1, 0, 0},
    {+90, 1, 0, 0},
    {  0, 0, 1, 0},
    {180, 0, 1, 0}
  };

  for (int i = 0; i < 6; ++i)
    m_rotatedSides[i] = new RotatedFrame(
      std::string(names[i]) + "_rotation",
      parentFrame(),
      Vec3(rotations[i][1], rotations[i][2], rotations[i][3]),
      deg2rad(rotations[i][0]));
  
  m_sides[0] = new TranslatedFrame(
      "front",
      m_rotatedSides[0],
      .5 * WEDGE_DEFAULT_LENGTH * Vec3::eZ());

  m_sides[1] = new TranslatedFrame(
      "back",
      m_rotatedSides[1],
      .5 * WEDGE_DEFAULT_LENGTH * Vec3::eZ());

  m_sides[2] = new TranslatedFrame(
      "right",
      m_rotatedSides[2],
      .5 * WEDGE_DEFAULT_WIDTH * Vec3::eZ());

  m_sides[3] = new TranslatedFrame(
      "left",
      m_rotatedSides[3],
      .5 * WEDGE_DEFAULT_WIDTH * Vec3::eZ());

  m_sides[4] = new TranslatedFrame(
      "top",
      m_rotatedSides[4],
      0);

  m_sides[5] = new TranslatedFrame(
      "bottom",
      m_rotatedSides[5],
      0);
}

WedgeElement::WedgeElement(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent)
  : Element(factory, name, frame, parent)
{
  m_cachedLength     = WEDGE_DEFAULT_LENGTH;
  m_cachedWidth      = WEDGE_DEFAULT_WIDTH;
  m_cachedHeights[0] = WEDGE_DEFAULT_HEIGHT1;
  m_cachedHeights[1] = WEDGE_DEFAULT_HEIGHT2;

  auto meanHeight = .5 * (m_cachedHeights[0] + m_cachedHeights[1]);
  auto maxHeight  = fmax(m_cachedHeights[0], m_cachedHeights[1]);

  setBoundingBox(
    Vec3(-m_cachedLength / 2, -m_cachedWidth/2, -meanHeight),
    Vec3(+m_cachedLength / 2, +m_cachedWidth/2, -meanHeight + maxHeight));

  initSides();

  m_wedge.setLength(m_cachedLength);
  m_wedge.setWidth(m_cachedWidth);
  m_wedge.setHeights(m_cachedHeights[0], m_cachedHeights[1]);
  
  addPort("front_side", m_sides[0]);
  addPort("back_side", m_sides[1]);

  addPort("right_side", m_sides[2]);
  addPort("left_side", m_sides[3]);

  addPort("top_side", m_sides[4]);
  addPort("bottom_side", m_sides[5]);

  refreshProperties();
}

void
WedgeElement::renderOpenGL()
{
  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_LINE_BIT);
    material("cube");
    m_wedge.display();
  glPopAttrib();
}


WedgeElement::~WedgeElement()
{
  for (int i = 0; i < 6; ++i) {
    delete m_sides[i];
    delete m_rotatedSides[i];
  }
}
