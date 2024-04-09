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
RayBeamElement::raysToVertices()
{
  size_t size = m_rays.size();
  GLfloat transp = sqrt(.125 * 250. / size);
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
RayBeamElement::setList(std::list<Ray> const &list)
{
  pthread_mutex_lock(&m_rayMutex);
  m_rays = list;
  raysToVertices();
  pthread_mutex_unlock(&m_rayMutex);
}

void
RayBeamElement::renderOpenGL()
{
  GLVectorStorage vec;

  pthread_mutex_lock(&m_rayMutex);

  glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glDisable(GL_LIGHTING);
  glDepthFunc(GL_ALWAYS);
  glEnable(GL_DEPTH_TEST);

  // THESE CANNOT BE OR'd
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

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
