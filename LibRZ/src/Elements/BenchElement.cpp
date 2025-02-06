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

#include <Elements/BenchElement.h>
#include <TranslatedFrame.h>
#include <GL/glut.h>

using namespace RZ;

#define BENCH_DEFAULT_WIDTH        2
#define BENCH_DEFAULT_DEPTH        1.5
#define BENCH_DEFAULT_TABLE_HEIGHT 0.03
#define BENCH_DEFAULT_LEG_RADIUS   0.15
#define BENCH_DEFAULT_LEG_SEP      (2 * BENCH_DEFAULT_LEG_RADIUS)

RZ_DESCRIBE_ELEMENT(BenchElement, "An optical bench with four legs")
{
  property("height", 1, "Height of the table [m]");
}

bool
BenchElement::propertyChanged(std::string const &name, PropertyValue const &val)
{
  if (name == "height") {
    Real height = val;
    m_cachedHeight = height;
    
    m_cylinder.setRadius(BENCH_DEFAULT_LEG_RADIUS);
    m_cylinder.setHeight(height - BENCH_DEFAULT_TABLE_HEIGHT);

    m_surfaceFrame->setDistance(height * Vec3::eZ());
    m_surfaceFrame->recalculate();

    setBoundingBox(
      Vec3(-BENCH_DEFAULT_WIDTH / 2, -BENCH_DEFAULT_DEPTH / 2, 0),
      Vec3(+BENCH_DEFAULT_WIDTH / 2, +BENCH_DEFAULT_DEPTH / 2, m_cachedHeight));

    return true;
  }

  return Element::propertyChanged(name, val);
}

BenchElement::BenchElement(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent)
  : Element(factory, name, frame, parent)
{
  m_cylinder.setVisibleCaps(true, false);

  m_surfaceFrame = registerPort(
    "surface",
    new TranslatedFrame("surface", frame, Vec3::zero()));

  refreshProperties();
}

void
BenchElement::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  
  if (role == "table") {
    glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.25, .25, .25));
    glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(.1, .1, .1));
  } else if (role == "legs") {
    glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.1, .1, .1));
    glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(.1, .1, .1));
  }
}


void
BenchElement::renderOpenGL()
{
  
  GLfloat legLoc[4][2] = {
    {
      -BENCH_DEFAULT_WIDTH / 2 + BENCH_DEFAULT_LEG_SEP, 
      -BENCH_DEFAULT_DEPTH / 2 + BENCH_DEFAULT_LEG_SEP
    },
    {
      +BENCH_DEFAULT_WIDTH / 2 - BENCH_DEFAULT_LEG_SEP, 
      -BENCH_DEFAULT_DEPTH / 2 + BENCH_DEFAULT_LEG_SEP
    },
    {
      -BENCH_DEFAULT_WIDTH / 2 + BENCH_DEFAULT_LEG_SEP, 
      +BENCH_DEFAULT_DEPTH / 2 - BENCH_DEFAULT_LEG_SEP
    },
    {
      +BENCH_DEFAULT_WIDTH / 2 - BENCH_DEFAULT_LEG_SEP, 
      +BENCH_DEFAULT_DEPTH / 2 - BENCH_DEFAULT_LEG_SEP
    }
  };
  // Draw the table itself

  material("table");

  glPushMatrix();
  glTranslatef(0, 0, m_cachedHeight - BENCH_DEFAULT_TABLE_HEIGHT / 2);
  glScalef(BENCH_DEFAULT_WIDTH, BENCH_DEFAULT_DEPTH, BENCH_DEFAULT_TABLE_HEIGHT);
  GLCube(1);
  glPopMatrix();

  // Draw the legs
  material("legs");

  for (auto i = 0; i < 4; ++i) {
    glPushMatrix();
    glTranslatef(legLoc[i][0], legLoc[i][1], 0);

    m_cylinder.display();

    glPopMatrix();
  }
}


BenchElement::~BenchElement()
{
}
