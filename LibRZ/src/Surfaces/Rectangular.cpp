#include <Surfaces/Rectangular.h>
#include <GLHelpers.h>

using namespace RZ;

RectangularFlatSurface::RectangularFlatSurface()
{
  m_edges.resize(1);
  m_edges[0].resize(3 * 4);

  updateEdges();
}

RectangularFlatSurface::~RectangularFlatSurface()
{
}

void
RectangularFlatSurface::updateEdges()
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
RectangularFlatSurface::edges() const
{
  return m_edges;
}

void
RectangularFlatSurface::setWidth(Real width)
{
  m_width = width;
  updateEdges();
}

void
RectangularFlatSurface::setHeight(Real height)
{
  m_height = height;
  updateEdges();
}

bool
RectangularFlatSurface::intercept(
  Vec3 &coord,
  Vec3 &n,
  Real &dt,
  Vec3 const &origin,
  Vec3 const &direction) const
{
  Real halfW  = .5 * m_width;
  Real halfH  = .5 * m_height;

  if (isZero(direction.z))
    return false;

  dt    = -origin.z / direction.z;

  if (dt < 0)
    return false;
  
  coord = origin + dt * direction;
  n     = Vec3::eZ();
  
  if (fabs(coord.x) < halfW && fabs(coord.y) < halfH)
    return !complementary();

  return complementary();
}

void
RectangularFlatSurface::generatePoints(
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
RectangularFlatSurface::area() const
{
  return m_width * m_height;
}

std::string
RectangularFlatSurface::name() const
{
  return "RectangularFlat";
}

void
RectangularFlatSurface::renderOpenGL()
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

