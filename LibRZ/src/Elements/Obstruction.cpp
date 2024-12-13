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

#define GL_GLEXT_PROTOTYPES

#include <Elements/Obstruction.h>
#include <TranslatedFrame.h>
#include <png++/png.hpp>
#include <Logger.h>

using namespace RZ;

static const char *g_vertexToTex = 
"#version 120\n"
"attribute vec3 verCoord;\n"
"attribute vec2 texCoord;\n"
"varying vec2 interp_texCoord;\n"
"void main()\n"
"{\n"
"  interp_texCoord = texCoord;\n"
"  gl_Position     = gl_ModelViewProjectionMatrix * vec4(verCoord, 1.0);\n"
"}\n";

static const char *g_texAlphaTest = 
  "#version 120\n"
  "varying vec2 interp_texCoord;\n"
  "uniform vec3 color;"
  "uniform sampler2D theTexture;\n"

  "void main()\n"
  "{\n"
  "  vec4 texel = texture2D(theTexture, interp_texCoord);\n"
  "  if (texel.a < 0.5)\n"
  "    discard;\n"
  "  gl_FragColor = vec4(color, 1.);\n"
  "}\n";

void
Obstruction::recalcModel()
{
  GLfloat fRadius = static_cast<GLfloat>(m_radius);

  m_processor->setRadius(m_radius);
  m_disc.setRadius(m_radius);

  if (m_cols > 0 && m_rows > 0) {
    m_halfMapWidth  = m_cols / fmax(m_cols, m_rows) * m_radius;
    m_halfMapHeight = m_rows / fmax(m_cols, m_rows) * m_radius;
  }

  m_processor->setObstructionMap(
    2 * m_halfMapWidth,
    2 * m_halfMapHeight,
    m_obstructionMap,
    m_cols,
    m_rows,
    m_stride);

  Vertex newVertices[4] = {
    {{+fRadius, -fRadius, 0}, {1, 1}},
    {{+fRadius, +fRadius, 0}, {1, 0}},
    {{-fRadius, +fRadius, 0}, {0, 0}},
    {{-fRadius, -fRadius, 0}, {0, 1}}
  };

  memcpy(m_obsVertices, newVertices, sizeof(newVertices));

  setBoundingBox(
      Vec3(-m_radius, -m_radius, 0),
      Vec3(+m_radius, +m_radius, 0));

  updatePropertyValue("radius",   m_radius);
  updatePropertyValue("diameter", 2 * m_radius);

  m_texDirty = true;
}

void
Obstruction::rebuildTexture()
{
  size_t size = m_obstructionMap.size();
  m_textureData.resize(size * 4);

  for (size_t i = 0; i < size; ++i) {
    m_textureData[4 * i + 0] = static_cast<uint8_t>(255 * m_obstructionMap[i]);
    m_textureData[4 * i + 1] = static_cast<uint8_t>(255 * m_obstructionMap[i]);
    m_textureData[4 * i + 2] = static_cast<uint8_t>(255 * m_obstructionMap[i]);
    m_textureData[4 * i + 3] = static_cast<uint8_t>(255 * m_obstructionMap[i]);
  }

  m_texDirty = true;
}

void
Obstruction::setFromPNG(std::string const &path)
{
  png::image<png::gray_pixel_16> inputMap(path);
  size_t allocSize;
  unsigned int i, j;
  unsigned int stride = inputMap.get_width();
  unsigned int rows, cols;

  cols = stride;
  rows = inputMap.get_height();

  allocSize = cols * rows;
  m_obstructionMap.resize(allocSize);
  m_textureData.clear();

  for (i = 0; i < rows; ++i) {
    for (j = 0; j < cols; ++j) {
      auto pixel = inputMap.get_pixel(j, i);

      m_obstructionMap[j + i * stride] = 1. - pixel / 65535.;
    }
  }

  m_rows   = rows;
  m_cols   = cols;
  m_stride = stride;

  rebuildTexture();
}

bool
Obstruction::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else if (name == "diameter") {
    m_radius = 0.5 * static_cast<Real>(value);
    recalcModel();
  } else if (name == "file") {
    std::string newPath = std::get<std::string>(value);
    if (m_path != newPath) {
      try {
        m_rows = m_cols = m_stride = 0;
        m_obstructionMap.clear();
        m_path.clear();
        if (!newPath.empty())
          setFromPNG(newPath);
        m_path = newPath;
        recalcModel();
      } catch (std::runtime_error const &e) {
        RZError("Cannot open obstruction file: %s\n", e.what());
      }
    }
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}


Obstruction::Obstruction(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new ObstructionProcessor;

  registerProperty("radius",    2.5e-2);
  registerProperty("diameter",  5e-2);
  registerProperty("file",      "");

  m_stopSurface = new TranslatedFrame("stopSurf", frame, Vec3::zero());

  pushOpticalSurface("stopSurf", m_stopSurface, m_processor);

  recalcModel();
}

Obstruction::~Obstruction()
{
  if (m_processor != nullptr)
    delete m_processor;

  if (m_alphaTestShader != nullptr)
    delete m_alphaTestShader;
}

void
Obstruction::nativeMaterialOpenGL(std::string const &)
{
  GLVectorStorage vec;
  GLfloat shiny = 0;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.1, .1, .1));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, 0, 0));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
Obstruction::initOpenGLObjects()
{
// Rebuild shader. TODO: Cache!
  if (m_alphaTestShader != nullptr) {
    delete m_alphaTestShader;
    m_alphaTestShader = nullptr;
  }

  m_alphaTestShader = new GLShader(g_vertexToTex, g_texAlphaTest);
  m_alphaTestShader->use();
  
  m_verCoordAttrib = 0;
  m_texCoordAttrib = 1;

  m_textureUniformId = glGetUniformLocation(
    m_alphaTestShader->program(),
    "theTexture");

  m_colorUniformId   = glGetUniformLocation(
    m_alphaTestShader->program(),
    "color");
  
  glActiveTexture(GL_TEXTURE0);

  // Allocate free texture ID
  glGenTextures(1, &m_textureId);

  glGenVertexArrays(1, &m_vaoId);
  glBindVertexArray(m_vaoId);
    glEnableVertexAttribArray(m_verCoordAttrib);
    glEnableVertexAttribArray(m_texCoordAttrib);
    // Allocate vertex buffer
    glGenBuffers(1, &m_vboId);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);
      glBufferData(GL_ARRAY_BUFFER, sizeof(m_obsVertices), 0, GL_STATIC_DRAW);
      // From this buffer, assign vertex coordinates to VAO.array[0]
      glVertexAttribPointer(
        m_verCoordAttrib,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const void *>(0)); // Hahaha

      // From this buffer, assign texture coordinates to VAO.array[1]
      glVertexAttribPointer(
        m_texCoordAttrib,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const void *>(3 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Allocate index buffer. Since this does not change, we can go ahead
    // and also upload them here
    glGenBuffers(1, &m_iboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboId);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_obsIndices), 0, GL_STATIC_DRAW);
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(m_obsIndices), m_obsIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(m_texCoordAttrib);
    glDisableVertexAttribArray(m_verCoordAttrib);
  glBindVertexArray(0);

  uploadAll();

  m_openGlInitilized = true;
}

void
Obstruction::enterOpenGL()
{
  if (!m_openGlInitilized)
    initOpenGLObjects();
}

void
Obstruction::uploadAll()
{
  // Define it as 2D
  glBindTexture(GL_TEXTURE_2D, m_textureId);

  // Configure texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  
  glTexImage2D(
    GL_TEXTURE_2D, 
    0, 
    GL_RGBA8, 
    m_cols, 
    m_rows, 
    0, 
    GL_RGBA, 
    GL_UNSIGNED_BYTE, 
    m_textureData.data());

  // Upload data to VBO
  glBindVertexArray(m_vaoId);
    glEnableVertexAttribArray(m_verCoordAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_obsVertices), m_obsVertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(m_verCoordAttrib);
  glBindVertexArray(0);

  m_texDirty = false;
}

void
Obstruction::renderOpenGL()
{
  if (m_rows == 0 || m_cols == 0) {
    material("input.obs");

    m_disc.display();
    glRotatef(180, 1, 0, 0);
    glTranslatef(0, 0, 1e-3 * m_radius);

    material("output.obs");
    m_disc.display();
  } else {
    if (!m_openGlInitilized)
      initOpenGLObjects();

    glActiveTexture(GL_TEXTURE0);

    if (m_texDirty)
      uploadAll();

    glPushAttrib(GL_ALL_ATTRIB_BITS);
      glBindTexture(GL_TEXTURE_2D, m_textureId);
      m_alphaTestShader->use();
        glUniform1i(m_textureUniformId, 0);
        glUniform3f(m_colorUniformId, red(), green(), blue());
        glBindVertexArray(m_vaoId);
          glEnableVertexAttribArray(m_verCoordAttrib);
          glEnableVertexAttribArray(m_texCoordAttrib);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboId);
            glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, nullptr);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          glDisableVertexAttribArray(m_texCoordAttrib);
          glDisableVertexAttribArray(m_verCoordAttrib);
        glBindVertexArray(0);
    m_alphaTestShader->leave();

    glPopAttrib();
  }
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
ObstructionFactory::name() const
{
  return "Obstruction";
}

Element *
ObstructionFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new Obstruction(this, name, pFrame, parent);
}
