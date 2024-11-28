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

#include <PhaseScreen.h>

#include <PhaseScreen.h>
#include <TranslatedFrame.h>
#include <Helpers.h>
#include "inferno.h"

using namespace RZ;

void
PhaseScreen::recalcModel()
{
  m_processor->setRadius(m_radius);
  m_processor->setRefractiveIndex(m_muIn, m_muOut);

  m_skyDiscFront.setRadius(m_radius);
  m_skyDiscBack.setRadius(m_radius);

  setBoundingBox(
      Vec3(-m_radius, -m_radius, 0),
      Vec3(+m_radius, +m_radius, 0));
}

void
PhaseScreen::recalcTexture()
{
  size_t pixels = m_textureData.size() / 3;

  Real max = 0;
  for (uint64_t i = 0; i < pixels; ++i) {
    Real rho   = (i >> 8) / 255.;
    Real alpha = (i & 0xff) / 255. * 2 * M_PI;
    Real x     = rho * cos(alpha);
    Real y     = rho * sin(alpha);
    Real v     = fabs(m_processor->Z(x, y));
    if (v > max)
      max = v;
  }

  if (max > 0) {
    for (uint64_t i = 0; i < pixels; ++i) {
      Real rho      = (i >> 8) / 255.;
      Real alpha    = (i & 0xff) / 255. * 2 * M_PI;
      Real x        = rho * cos(alpha);
      Real y        = rho * sin(alpha);
      Real v        = fabs(m_processor->Z(x, y));
      uint8_t index = v * 255 / max;

      m_textureData[3 * i + 0] = 255 * g_inferno[index][0];
      m_textureData[3 * i + 1] = 255 * g_inferno[index][1];
      m_textureData[3 * i + 2] = 255 * g_inferno[index][2];
    }
  } else {
    for (uint64_t i = 0; i < pixels; ++i) {
      m_textureData[3 * i + 0] = 255 * g_inferno[0][0];
      m_textureData[3 * i + 1] = 255 * g_inferno[0][1];
      m_textureData[3 * i + 2] = 255 * g_inferno[0][2];
    }
  }

  m_texDirty = true;
}

bool
PhaseScreen::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  unsigned int zCoef;
  
  if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else if (name == "ni") {
    m_muIn = value;
    recalcModel();
  } else if (name == "no") {
    m_muOut = value;
    recalcModel();
  } else if (sscanf(name.c_str(), "Z%u", &zCoef) == 1) {
    Real asReal = value;
    if (!releq(m_processor->coef(zCoef), asReal)) {
      m_processor->setCoef(zCoef, asReal);
      recalcTexture();
    }
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}


PhaseScreen::PhaseScreen(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new PhaseScreenProcessor;

  unsigned int i;

  for (i = 0; i < 60; ++i)
    registerProperty(string_printf("Z%u", i), 0.);
  
  registerProperty("radius",    2.5e-2);
  registerProperty("ni",        1);
  registerProperty("no",        1.5);
  m_stopSurface = new TranslatedFrame("refSurf", frame, Vec3::zero());

  pushOpticalSurface("stopSurf", m_stopSurface, m_processor);

  m_textureData.resize(3 * 256 * 256);

  m_skyDiscFront.setSlices(64);
  m_skyDiscBack.setSlices(64);
  m_skyDiscBack.setInverted(true);

  recalcModel();
  recalcTexture();
}

void
PhaseScreen::uploadTexture()
{
  // Define it as 2D
  glBindTexture(GL_TEXTURE_2D, m_textureId);

  // Configure texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  
  glTexImage2D(
    GL_TEXTURE_2D, 
    0, 
    GL_RGB8, 
    256, 
    256, 
    0, 
    GL_RGB, 
    GL_UNSIGNED_BYTE, 
    m_textureData.data());

  m_texDirty = false;
}

PhaseScreen::~PhaseScreen()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
PhaseScreen::enterOpenGL()
{
  // Allocate free texture ID
  glGenTextures(1, &m_textureId);

  uploadTexture();
}

void
PhaseScreen::nativeMaterialOpenGL(std::string const &)
{
  GLVectorStorage vec;
  GLfloat shiny = 0;
  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.1, .1, .1));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, 0, 0));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
PhaseScreen::renderOpenGL()
{
  GLVectorStorage vec;

  if (m_texDirty)
    uploadTexture();

  glEnable(GL_TEXTURE_2D);
    glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(1.0, 1.0, 1.0));
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    m_skyDiscBack.display();
  glDisable(GL_TEXTURE_2D);

  glTranslatef(0, 0, 1e-3 * m_radius);
  
  glEnable(GL_TEXTURE_2D);
    glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(1.0, 1.0, 1.0));
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    m_skyDiscFront.display();
  glDisable(GL_TEXTURE_2D);
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
PhaseScreenFactory::name() const
{
  return "PhaseScreen";
}

Element *
PhaseScreenFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new PhaseScreen(this, name, pFrame, parent);
}
