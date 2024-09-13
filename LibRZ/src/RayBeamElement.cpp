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

#include <RayBeamElement.h>

using namespace RZ;

RayColoring RayBeamElement::m_defaultColoring;

void
RayColoring::id2color(uint32_t id, GLfloat *rgb) const
{
  // Yellow
  rgb[0] = 1;
  rgb[1] = 1;
  rgb[2] = 0;
}

void
RayColoring::id2color(uint32_t id, GLfloat alpha, GLfloat *rgb) const
{
  id2color(id, rgb);
  rgb[3] = alpha;
}

void
PaletteBasedColoring::id2color(uint32_t id, GLfloat *rgb) const
{
  auto it = m_colors.find(id);

  if (it == m_colors.cend())
    memcpy(rgb, m_defaultColor, 3 * sizeof(GLfloat));
  else
    memcpy(rgb, it->second.rgb, 3 * sizeof(GLfloat));
}

void
PaletteBasedColoring::setColor(uint32_t id, Real r, Real g, Real b)
{
  CrappyCplusplusColorWrapper wrapper;
  GLfloat rgb[3] = {
    static_cast<GLfloat>(r),
    static_cast<GLfloat>(g),
    static_cast<GLfloat>(b)
  };

  memcpy(wrapper.rgb, rgb, sizeof(rgb));

  m_colors[id] = wrapper;
}

void
PaletteBasedColoring::setDefaultColor(Real r, Real g, Real b)
{
  m_defaultColor[0] = r;
  m_defaultColor[1] = g;
  m_defaultColor[2] = b;
}

void
RayBeamElement::raysToVertices()
{
  size_t size = m_rays.size();
  GLfloat transp = m_dynamicAlpha ? sqrt(.125 * 250. / size) : 1;
  size_t i = 0, j = 0;

  if (transp > 1)
    transp = 1;
  
  m_vertices.resize(6 * size);
  m_colors.resize(8 * size);

  for (auto p = m_rays.begin(); p != m_rays.end(); ++p) {
    Vec3 destination = p->origin + p->length * p->direction;
    m_vertices[i++]  = p->origin.x;
    m_vertices[i++]  = p->origin.y;
    m_vertices[i++]  = p->origin.z;

    m_vertices[i++]  = destination.x;
    m_vertices[i++]  = destination.y;
    m_vertices[i++]  = destination.z;
    
    m_rayColoring->id2color(p->id, transp, &m_colors[j]);
    memcpy(&m_colors[j + 4], &m_colors[j], 4 * sizeof(GLfloat));
    j += 8;
  }
}

RayBeamElement::RayBeamElement(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
: Element(factory, name, pFrame, parent)
{
  m_rayColoring = &m_defaultColoring;
}

RayBeamElement::~RayBeamElement()
{
  pthread_mutex_destroy(&m_rayMutex);
}

void
RayBeamElement::setRayColoring(RayColoring const *coloring)
{
  if (coloring == nullptr)
    coloring = &m_defaultColoring;
  
  if (m_rayColoring != coloring) {
    m_rayColoring = coloring;
    raysToVertices();
  }
}

void
RayBeamElement::setRayColoring(RayColoring const &ref)
{
  setRayColoring(&ref);
}

void
RayBeamElement::setDynamicAlpha(bool alpha)
{
  if (m_dynamicAlpha != alpha) {
    m_dynamicAlpha = alpha;
    raysToVertices();
  }
  
}

void
RayBeamElement::clear()
{
  setList(std::list<Ray>());
}

void
RayBeamElement::setList(std::list<Ray> const &list)
{
  pthread_mutex_lock(&m_rayMutex);
  m_rays = list;
  raysToVertices();
  pthread_mutex_unlock(&m_rayMutex);
}

void
RayBeamElement::setRayWidth(Real width)
{
  m_lineWidth = width;
}

void
RayBeamElement::renderOpenGL()
{
  GLVectorStorage vec;

  pthread_mutex_lock(&m_rayMutex);

  glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT);

  glDisable(GL_LIGHTING);

  if (m_dynamicAlpha)
    glDepthFunc(GL_ALWAYS);
  glEnable(GL_DEPTH_TEST);

  // THESE CANNOT BE OR'd
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glLineWidth(m_lineWidth);

  glColorPointer(4, GL_FLOAT, 4 * sizeof(GLfloat), m_colors.data());
  glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_vertices.data());
  glDrawArrays(GL_LINES, 0, m_vertices.size() / 3);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  glPopAttrib();
  pthread_mutex_unlock(&m_rayMutex);
}

/////////////////////////////////// Factory ////////////////////////////////////
std::string
RayBeamElementFactory::name() const
{
  return "RayBeam";
}

Element *
RayBeamElementFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new RayBeamElement(this, name, pFrame, parent);
}
