#include <RayBeamElement.h>

using namespace RZ;

void
RayBeamElement::raysToVertices()
{
  m_vertices.clear();

  for (auto p = m_rays.begin(); p != m_rays.end(); ++p) {
    Vec3 destination = p->origin + p->length * p->direction;
    m_vertices.push_back(p->origin.x);
    m_vertices.push_back(p->origin.y);
    m_vertices.push_back(p->origin.z);

    m_vertices.push_back(destination.x);
    m_vertices.push_back(destination.y);
    m_vertices.push_back(destination.z);
  }
}

RayBeamElement::RayBeamElement(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
: Element(factory, name, pFrame, parent)
{

}

RayBeamElement::~RayBeamElement()
{
  pthread_mutex_destroy(&m_rayMutex);
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
  size_t size = m_vertices.size() / 6;
  GLfloat transp = sqrt(.125 * 250. / size);

  glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  if (transp > 1)
    transp = 1;
  glColor4f(1.0, 1.0, 0.0, transp);
  glDisable(GL_LIGHTING);
  glDepthFunc(GL_ALWAYS);
  glEnable(GL_DEPTH_TEST);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, m_vertices.data());
  glDrawArrays(GL_LINES, 0, m_vertices.size() / 3);
  glDisableClientState(GL_VERTEX_ARRAY);

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
