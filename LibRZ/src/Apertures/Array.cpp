#include <Apertures/Array.h>

using namespace RZ;

void
ApertureArray::recalculateDimensions()
{
  m_subApertureWidth  = m_width / m_cols;
  m_subApertureHeight = m_height / m_rows;
}

ApertureArray::ApertureArray(GenericAperture *ap)
{
  m_subAperture = ap;
  recalculateDimensions();
}

ApertureArray::~ApertureArray()
{
  if (m_subAperture != nullptr)
    delete m_subAperture;
}

void
ApertureArray::setWidth(Real width)
{
  m_width = width;
  recalculateDimensions();
}

void
ApertureArray::setHeight(Real height)
{
  m_height = height;
  recalculateDimensions();
}

void
ApertureArray::setCols(unsigned cols)
{
  m_cols = cols;
  recalculateDimensions();
}

void
ApertureArray::setRows(unsigned rows)
{
  m_rows = rows;
  recalculateDimensions();
}

bool
ApertureArray::intercept(
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
ApertureArray::generatePoints(
    const ReferenceFrame *,
    Real *pointArr,
    unsigned int N)
{
  
}
    