#include <Apertures/Rectangular.h>
#include <GLHelpers.h>

using namespace RZ;

RectangularAperture::RectangularAperture()
{
  m_edges.resize(1);
  m_edges[0].resize(3 * 4);

  updateEdges();
}

RectangularAperture::~RectangularAperture()
{
}

void
RectangularAperture::updateEdges()
{
  unsigned int p;
  Real x[] = {m_width / 2, -m_width / 2, -m_width / 2, m_width / 2};
  Real y[] = {m_height / 2, m_height / 2, -m_height / 2, -m_height / 2};

  for (auto i = 0; i < 4; ++i) {
    m_edges[0][3 * i + 0] = x[i];
    m_edges[0][3 * i + 1] = y[i];
    m_edges[0][3 * i + 2] = 0;
  }
}

std::vector<std::vector<Real>> const &
RectangularAperture::edges() const
{
  return m_edges;
}

void
RectangularAperture::setWidth(Real width)
{
  m_width = width;
  updateEdges();
}

void
RectangularAperture::setHeight(Real height)
{
  m_height = height;
  updateEdges();
}

bool
RectangularAperture::intercept(
  Vec3 &coord,
  Vec3 &n,
  Real &deltaT,
  Vec3 const &origin) const
{
  Real halfW  = .5 * m_width;
  Real halfH  = .5 * m_height;

  return fabs(coord.x) < halfW && fabs(coord.y) < halfH;
}

void
RectangularAperture::generatePoints(
    const ReferenceFrame *frame,
    Real *pointArr,
    Real *normalArr,
    unsigned int N)
{
  unsigned int i;
  auto &state = randState();

  for (i = 0; i < N; ++i) {
    Real y = m_height * (state.randu() - .5);
    Real x = m_width  * (state.randu() - .5);
    
    frame->fromRelative(Vec3(x, y, 0)).copyToArray(pointArr + 3 * i);
    frame->fromRelativeVec(Vec3(0, 0, 1)).copyToArray(normalArr + 3 * i);
  }
}

Real
RectangularAperture::area() const
{
  return m_width * m_height;
}

void
RectangularAperture::renderOpenGL()
{
  glBegin(GL_LINE_LOOP);
    glVertex3f(-m_width / 2, -m_height / 2, 0);
    glVertex3f(+m_width / 2, -m_height / 2, 0);
    glVertex3f(+m_width / 2, +m_height / 2, 0);
    glVertex3f(-m_width / 2, +m_height / 2, 0);
  glEnd();

  glBegin(GL_LINES);
    glVertex3f(-m_width / 2, 0, 0);
    glVertex3f(+m_width / 2, 0, 0);
    glVertex3f(0, -m_height / 2, 0);
    glVertex3f(0, +m_height / 2, 0);
  glEnd();
}

