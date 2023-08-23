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

}

void
RayBeamElement::setList(std::list<Ray> const &list)
{
  m_rays = list;
  raysToVertices();
}

void
RayBeamElement::renderOpenGL()
{
  GLVectorStorage vec;
  GLfloat transp = .125 * 2500. / m_rays.size();

  glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  if (transp > 1)
    transp = 1;
  glColor4f(1.0, 1.0, 0.0, transp);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, m_vertices.data());
  glDrawArrays(GL_LINES, 0, m_vertices.size() / 3);
  glDisableClientState(GL_VERTEX_ARRAY);

  glPopAttrib();
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
