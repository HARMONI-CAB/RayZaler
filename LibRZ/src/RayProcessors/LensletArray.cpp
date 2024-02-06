#include <RayProcessors/LensletArray.h>
#include <Apertures/Spherical.h>
#include <ReferenceFrame.h>

using namespace RZ;

// In the lenslet array we basically have an array of partially overlapped
// convex surfaces.

LensletArrayProcessor::LensletArrayProcessor()
{
  defineAperture(new SphericalAperture(m_lensletRadius, m_rCurv));

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

    aperture<SphericalAperture>()->setRadius(m_lensletRadius);
    aperture<SphericalAperture>()->setCurvatureRadius(m_rCurv);
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
  aperture<SphericalAperture>()->setConvex(convex);
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
  Vec3 normal;
  Real halfW  = .5 * m_width;
  Real halfH  = .5 * m_height;
  
  for (i = 0; i < count; ++i) {
    Vec3 origin = plane->toRelative(Vec3(beam.origins + 3 * i));
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));

    if (fabs(coord.x) < halfW && fabs(coord.y) < halfH) {
      // Determine which lenslet this ray belongs to
      unsigned col = floor((coord.x + halfW) / m_lensletWidth);
      unsigned row = floor((coord.y + halfH) / m_lensletHeight);

      // Make coord relative to the lenslet center
      Real lensOX = - halfW + (col + .5) * m_lensletWidth;
      Real lensOY = - halfH + (row + .5) * m_lensletHeight;

      coord.x  -= lensOX;
      coord.y  -= lensOY;

      origin.x -= lensOX;
      origin.y -= lensOY;

      if (aperture()->intercept(coord, normal, origin)) {
        // Readjust center
        coord.x  += lensOX;
        coord.y  += lensOY;

        origin.x += lensOX;
        origin.y += lensOY;

        plane->fromRelative(coord).copyToArray(beam.destinations + 3 * i);
        snell(
          Vec3(beam.directions + 3 * i), 
          plane->fromRelativeVec(normal),
          m_IOratio).copyToArray(beam.directions + 3 * i);
      } else {
        beam.prune(i);
      }
    } else {
      // Outside lens
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));

  beam.n = m_muIn;
}
