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

#include <Elements/PhaseScreen.h>
#include <TranslatedFrame.h>
#include <Helpers.h>
#include "inferno.h"

using namespace RZ;

RZ_DESCRIBE_OPTICAL_ELEMENT(PhaseScreen, "Circular phase screen with irregular height described through Zernike polynomials")
{
  unsigned int i;

  for (i = 0; i < 60; ++i) {
    property(
      string_printf("Z%u", i), 
      0.,
      string_printf("Coefficient for polynomial $Z_{%d}$ [m]", i));
  }

  property("radius",    2.5e-2, "Radius of the phase screen [m]");
  property("diameter",    5e-2, "Diameter of the phase screen [m]");
  property("ni",            1., "Input refractive index");
  property("no",           1.5, "Output refractive index");
}

void
PhaseScreen::recalcModel()
{
  m_boundary->setRadius(m_radius);
  m_boundary->setRefractiveIndex(m_muIn, m_muOut);

  m_skyDiscFront.setRadius(m_radius);
  m_skyDiscBack.setRadius(m_radius);

  setBoundingBox(
      Vec3(-m_radius, -m_radius, 0),
      Vec3(+m_radius, +m_radius, 0));

  updatePropertyValue("radius",   m_radius);
  updatePropertyValue("diameter", 2 * m_radius);
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
    Real v     = fabs(m_boundary->Z(x, y));
    if (v > max)
      max = v;
  }

  if (max > 0) {
    for (uint64_t i = 0; i < pixels; ++i) {
      Real rho      = (i >> 8) / 255.;
      Real alpha    = (i & 0xff) / 255. * 2 * M_PI;
      Real x        = rho * cos(alpha);
      Real y        = rho * sin(alpha);
      Real v        = fabs(m_boundary->Z(x, y));
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
  } else if (name == "diameter") {
    m_radius = 0.5 * static_cast<Real>(value);
    recalcModel();
  } else if (name == "ni") {
    m_muIn = value;
    recalcModel();
  } else if (name == "no") {
    m_muOut = value;
    recalcModel();
  } else if (sscanf(name.c_str(), "Z%u", &zCoef) == 1) {
    Real asReal = value;
    if (!releq(m_boundary->coef(zCoef), asReal)) {
      m_boundary->setCoef(zCoef, asReal);
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
  m_boundary = new PhaseScreenBoundary;

  m_tSurface = new TranslatedFrame("interface", frame, Vec3::zero());

  pushOpticalSurface("interface", m_tSurface, m_boundary);
  addPort("aperture", m_tSurface);

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
  if (m_boundary != nullptr)
    delete m_boundary;
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
