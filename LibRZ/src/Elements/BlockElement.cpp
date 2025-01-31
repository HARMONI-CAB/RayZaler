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

#include <Elements/BlockElement.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <GL/glut.h>

using namespace RZ;

#define BLOCK_DEFAULT_LENGTH 1.
#define BLOCK_DEFAULT_WIDTH  1.
#define BLOCK_DEFAULT_HEIGHT 1.

RZ_DESCRIBE_ELEMENT(BlockElement, "Rectangular prism with adjustable dimensions")
{
  property("length",    BLOCK_DEFAULT_LENGTH, "Length of the block [m]");
  property("width",     BLOCK_DEFAULT_WIDTH,  "Width of the block [m]");
  property("height",    BLOCK_DEFAULT_HEIGHT, "Height of the block [m]");
  property("wireFrame", false,                "Represent as wireframe only");
}

bool
BlockElement::propertyChanged(std::string const &name, PropertyValue const &val)
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
  } else if (name == "height") {
    m_sides[4]->setDistance(.5 * value * Vec3::eZ());
    m_sides[5]->setDistance(.5 * value * Vec3::eZ());
    m_sides[4]->recalculate();
    m_sides[5]->recalculate();
    m_cachedHeight = value;
    hadIt = true;
  } else if (name == "wireFrame") {
    m_wireFrame = value > .5;
    hadIt = true;
  }
  
  if (hadIt) {
    setBoundingBox(
      Vec3(-m_cachedLength / 2, -m_cachedWidth/2, -m_cachedHeight/2),
      Vec3(+m_cachedLength / 2, +m_cachedWidth/2, +m_cachedHeight/2));

    return true;
  }
  
  
  return Element::propertyChanged(name, val);
}

void
BlockElement::initSides()
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
      .5 * BLOCK_DEFAULT_LENGTH * Vec3::eZ());

  m_sides[1] = new TranslatedFrame(
      "back",
      m_rotatedSides[1],
      .5 * BLOCK_DEFAULT_LENGTH * Vec3::eZ());

  m_sides[2] = new TranslatedFrame(
      "right",
      m_rotatedSides[2],
      .5 * BLOCK_DEFAULT_WIDTH * Vec3::eZ());

  m_sides[3] = new TranslatedFrame(
      "left",
      m_rotatedSides[3],
      .5 * BLOCK_DEFAULT_WIDTH * Vec3::eZ());

  m_sides[4] = new TranslatedFrame(
      "top",
      m_rotatedSides[4],
      .5 * BLOCK_DEFAULT_HEIGHT * Vec3::eZ());

  m_sides[5] = new TranslatedFrame(
      "bottom",
      m_rotatedSides[5],
      .5 * BLOCK_DEFAULT_HEIGHT * Vec3::eZ());
}

BlockElement::BlockElement(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent)
  : Element(factory, name, frame, parent)
{
  m_cachedLength = BLOCK_DEFAULT_LENGTH;
  m_cachedWidth  = BLOCK_DEFAULT_WIDTH;
  m_cachedHeight = BLOCK_DEFAULT_HEIGHT;

  setBoundingBox(
      Vec3(-m_cachedLength / 2, -m_cachedWidth/2, -m_cachedHeight/2),
      Vec3(+m_cachedLength / 2, +m_cachedWidth/2, +m_cachedHeight/2));

  initSides();

  addPort("front_side", m_sides[0]);
  addPort("back_side", m_sides[1]);

  addPort("right_side", m_sides[2]);
  addPort("left_side", m_sides[3]);

  addPort("top_side", m_sides[4]);
  addPort("bottom_side", m_sides[5]);

  refreshProperties();
}

void
BlockElement::renderOpenGL()
{
  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_LINE_BIT);
    if (m_wireFrame) {
      glEnable(GL_COLOR);
      glDisable(GL_LIGHTING);
      glLineWidth(2);
      glColor3f(red(), green(), blue());
    } else {
      material("cube");
    }

    glPushMatrix();
    glScalef(m_cachedLength, m_cachedWidth, m_cachedHeight);
    GLCube(1, m_wireFrame);
    glPopMatrix();
  glPopAttrib();
}


BlockElement::~BlockElement()
{
  for (int i = 0; i < 6; ++i) {
    delete m_sides[i];
    delete m_rotatedSides[i];
  }
}
