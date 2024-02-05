#include <RayProcessors/LensletArray.h>
#include <ReferenceFrame.h>

using namespace RZ;

// In the lenslet array we basically have an array of partially overlapped
// convex surfaces.

LensletArrayProcessor::LensletArrayProcessor()
{
  recalculateDimensions();
}

void
LensletArrayProcessor::recalculateDimensions()
{
  if (m_dirty) {
    m_lensletWidth  = m_width / m_cols;
    m_lensletHeight = m_height / m_rows;
    m_lensletRadius = 
      .5 * sqrt(
        m_lensletWidth * m_lensletWidth + m_lensletHeight * m_lensletHeight);

    // Calculate curvature center
    m_center = sqrt(m_rCurv * m_rCurv - m_lensletRadius * m_lensletRadius);
    m_IOratio = m_muIn / m_muOut;
    m_dirty = false;
  }
}

void
LensletArrayProcessor::setCurvatureRadius(Real rCurv)
{
  m_rCurv = rCurv;
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setRefractiveIndex(Real in, Real out)
{
  m_muIn  = in;
  m_muOut = out;
  m_dirty = true;
  recalculateDimensions();
}
      
std::string
LensletArrayProcessor::name() const
{
  return "LensletArrayProcessor";
}

void
LensletArrayProcessor::setWidth(Real width)
{
  m_width = width;
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setHeight(Real height)
{
  m_height = height;
  m_dirty  = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setCols(unsigned cols)
{
  m_cols  = cols;
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setConvex(bool convex)
{
  m_convex = convex;
}

void
LensletArrayProcessor::setRows(unsigned rows)
{
  m_rows  = rows;
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Vec3 center  = plane->getCenter();
  Vec3 tX      = plane->eX();
  Vec3 tY      = plane->eY();
  Vec3 normal  = plane->eZ();
  Real Rcurv   = m_rCurv;
  Real sign    = m_convex ? +1 : -1;
  Vec3 Csphere = center + sign * m_center * normal;
  Real Rsq     = m_lensletRadius * m_lensletRadius;
  Real t;

  Real halfW  = .5 * m_width;
  Real halfH  = .5 * m_height;
  
  for (i = 0; i < count; ++i) {
    Vec3 coord  = Vec3(beam.destinations + 3 * i) - plane->getCenter();
    Real coordX = coord * tX;
    Real coordY = coord * tY;

    if (fabs(coordX) < halfW && fabs(coordY) < halfH) {
      // Determine which lenslet this ray belongs to
      unsigned col = floor((coordX + halfW) / m_lensletWidth);
      unsigned row = floor((coordY + halfH) / m_lensletHeight);

      // Calculate lenslet center
      Real lensOX = - halfW + (col + .5) * m_lensletWidth;
      Real lensOY = - halfH + (row + .5) * m_lensletHeight;

      // TODO: Abstract this smh. This is going to be used everywhere 
      // Adjust origin to fit the template lens 
      Vec3 lensOffset = lensOX * tX + lensOY * tY;
      Vec3 O  = Vec3(beam.origins + 3 * i) - lensOffset;
      Vec3 OC = O - Csphere;
      Vec3 u(beam.directions + 3 * i);
      Real uOC   = u * OC;
      Real Delta = uOC * uOC - OC * OC + Rcurv * Rcurv;

      if (Delta < 0) {
        beam.prune(i);
      } else {
        Real sqDelta = sqrt(Delta);
        t = m_convex ? -uOC - sqDelta : -uOC + sqDelta;
        
        // Calculation of lens intersection
        Vec3 destination(O + t * u);
        Vec3 normal = (destination - Csphere).normalized();
        
        if (!m_convex)
          normal = -normal;
        Vec3 nXu    = m_IOratio * normal.cross(u);

        destination += lensOffset;
        destination.copyToArray(beam.destinations + 3 * i);

        // Some Snell
        u  = -normal.cross(nXu) - normal * sqrt(1 - nXu * nXu);
        u.copyToArray(beam.directions + 3 * i);
      }
    } else {
      // Outside lens
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));

  beam.n = m_muIn;
}
