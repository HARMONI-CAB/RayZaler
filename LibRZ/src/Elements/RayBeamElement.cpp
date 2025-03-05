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

#include <Elements/RayBeamElement.h>

using namespace RZ;

RayColoring RayBeamElement::m_defaultColoring;

RZ_DESCRIBE_ELEMENT(RayBeamElement, "A beam of light composed of several rays")
{
}

void
RayColoring::id2color(uint32_t, GLfloat *rgb) const
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

RayColoring::~RayColoring()
{

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
LineVertexSet::renderOpenGL()
{
  // THESE CANNOT BE OR'd
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glLineWidth(lineWidth);

  glLineStipple(1, stipple);
  glEnable(GL_LINE_STIPPLE);
  
  glColorPointer(4, GL_FLOAT, 4 * sizeof(GLfloat), colors.data());
  glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), vertices.data());
  glDrawArrays(GL_LINES, 0, vertices.size() / 3);

  glDisable(GL_LINE_STIPPLE);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
}

void
LineVertexSet::clear()
{
  vertices.clear();
  colors.clear();
}

void
LineVertexSet::push(
  Vec3 const &origin,
  Vec3 const &dest,
  const GLfloat *color,
  const GLfloat *color2)
{
  size_t p = vertices.size();

  vertices.resize(p + 6);

  vertices[p++]  = origin.x;
  vertices[p++]  = origin.y;
  vertices[p++]  = origin.z;

  vertices[p++]  = dest.x;
  vertices[p++]  = dest.y;
  vertices[p++]  = dest.z;

  p = colors.size();
  colors.resize(p + 8);

  if (color2 == nullptr)
    color2 = color;

  memcpy(&colors[p],     color,  4 * sizeof(GLfloat));
  memcpy(&colors[p + 4], color2, 4 * sizeof(GLfloat));
}

void
RayBeamElement::raysToVertices()
{
  size_t size = m_rays.size();
  size_t actualCount = 0;
  GLfloat transp = m_dynamicAlpha ? sqrt(.125 * 250. / size) : 1;
  GLfloat black[4] = {0, 0, 0, 1.};

  uint32_t currId = 0;
  GLfloat currColor[4];
  bool tooMany = m_rays.size() > m_maxRays;
  Real drawP = 1;
  Real length;

  if (transp > 1)
    transp = 1;
  
  if (tooMany)
    drawP = static_cast<Real>(m_maxRays) / m_rays.size();

  m_commonRayVert.clear();
  m_chiefRayVert.clear();

  m_rayColoring->id2color(currId, transp, currColor);

  m_strayRays = 0;

  for (auto p = m_rays.begin(); p != m_rays.end(); ++p) {
    if (!p->intercepted) {
      length = fmax(p->length, p->cumOptLength / p->refNdx);
      ++m_strayRays;
    } else {
      length = p->length;
    }

    if (tooMany && drawP < m_randState.randu())
      continue;
    
    Vec3 destination = p->origin + length * p->direction;
    LineVertexSet *set = p->chief ? &m_chiefRayVert : &m_commonRayVert;

    if (p->id != currId) {
      currId = p->id;
      m_rayColoring->id2color(currId, transp, currColor);
    }

    set->push(
      p->origin,
      destination,
      currColor,
      p->intercepted ? nullptr : black);
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
  m_chiefRayVert.stipple = 0xff3c;
  m_chiefRayVert.lineWidth = 2;
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
  m_commonRayVert.lineWidth = width;
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

  m_chiefRayVert.renderOpenGL();
  m_commonRayVert.renderOpenGL();

  glPopAttrib();
  pthread_mutex_unlock(&m_rayMutex);
}
