#include <Surfaces/Array.h>
#include <GLHelpers.h>

using namespace RZ;

void
SurfaceArray::recalculateDimensions()
{
  m_subApertureWidth  = m_width / m_cols;
  m_subApertureHeight = m_height / m_rows;

  m_edges.clear();

  unsigned int i, j;
  Real halfW  = .5 * m_width;
  Real halfH  = .5 * m_height;

  for (j = 0; j < m_rows; ++j) {
    for (i = 0; i < m_cols; ++i) {
      Real lensOX = - halfW + (i + .5) * m_subApertureWidth;
      Real lensOY = - halfH + (j + .5) * m_subApertureHeight;

      for (auto &edge : m_subAperture->edges()) {
        std::vector<Real> locations;
        unsigned p = 0;

        for (auto k = 0; k < edge.size(); k += 3) {
          locations.push_back(edge[k + 0] + lensOX);
          locations.push_back(edge[k + 1] + lensOY);
          locations.push_back(edge[k + 2]);
        }
        
        m_edges.push_back(locations);
      }
    }
  }
}

SurfaceArray::SurfaceArray(SurfaceShape *ap)
{
  m_subAperture = ap;
  recalculateDimensions();
}

std::vector<std::vector<Real>> const &
SurfaceArray::edges() const
{
  return m_edges;
}

SurfaceArray::~SurfaceArray()
{
  if (m_subAperture != nullptr)
    delete m_subAperture;
}

void
SurfaceArray::setWidth(Real width)
{
  m_width = width;
  recalculateDimensions();
}

void
SurfaceArray::setHeight(Real height)
{
  m_height = height;
  recalculateDimensions();
}

void
SurfaceArray::setCols(unsigned cols)
{
  m_cols = cols;
  recalculateDimensions();
}

void
SurfaceArray::setRows(unsigned rows)
{
  m_rows = rows;
  recalculateDimensions();
}

bool
SurfaceArray::intercept(
  Vec3 &coord,
  Vec3 &n,
  Real &deltaT,
  Vec3 const &origin) const
{
  Vec3 relOrg, relCrd;
  Real halfW  = .5 * m_width;
  Real halfH  = .5 * m_height;

  if (fabs(coord.x) < halfW && fabs(coord.y) < halfH) {
    // Determine which lenslet this ray belongs to
    unsigned col = floor((coord.x + halfW) / m_subApertureWidth);
    unsigned row = floor((coord.y + halfH) / m_subApertureHeight);

    // Make coord relative to the lenslet center
    Real lensOX = - halfW + (col + .5) * m_subApertureWidth;
    Real lensOY = - halfH + (row + .5) * m_subApertureHeight;

    relCrd    = coord;
    relCrd.x -= lensOX;
    relCrd.y -= lensOY;
    
    relOrg    = origin;
    relOrg.x -= lensOX;
    relOrg.y -= lensOY;

    if (subAperture()->intercept(relCrd, n, deltaT, relOrg)) {
      // Readjust center
      coord     = relCrd;
      coord.x  += lensOX;
      coord.y  += lensOY;
      return true;
    }
  }

  return false;
}

void
SurfaceArray::generatePoints(
    const ReferenceFrame *,
    Real *pointArr,
    Real *normalArr,
    unsigned int N)
{
  // TODO
}

Real
SurfaceArray::area() const
{
  // TODO
  return m_width * m_height;
}

void
SurfaceArray::renderOpenGL()
{
  unsigned int i, j;
  Real halfW  = .5 * m_width;
  Real halfH  = .5 * m_height;

  for (j = 0; j < m_rows; ++j) {
    for (i = 0; i < m_cols; ++i) {
      Real lensOX = - halfW + (i + .5) * m_subApertureWidth;
      Real lensOY = - halfH + (j + .5) * m_subApertureHeight;

      glPushMatrix();
        glTranslatef(lensOX, lensOY, 0);
        m_subAperture->renderOpenGL();
      glPopMatrix();
    }
  }
}

