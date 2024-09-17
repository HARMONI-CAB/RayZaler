#include <RayProcessors/LensletArray.h>
#include <Apertures/Spherical.h>
#include <ReferenceFrame.h>
#include <Apertures/Array.h>
#include <Apertures/Spherical.h>

using namespace RZ;

// In the lenslet array we basically have an array of partially overlapped
// convex surfaces.

LensletArrayProcessor::LensletArrayProcessor()
{
  defineAperture(
    new ApertureArray(
      new SphericalAperture(1e-2, m_rCurv)));

  recalculateDimensions();
}

void
LensletArrayProcessor::recalculateDimensions()
{
  if (m_dirty) {
    auto *array   = aperture<ApertureArray>();
    auto *lenslet = array->subAperture<SphericalAperture>();

    Real lensletWidth  = array->subApertureWidth();
    Real lensletHeight = array->subApertureHeight();
    Real radius = 
      .5 * sqrt(
        lensletWidth * lensletWidth + lensletHeight * lensletHeight);

    // Calculate curvature center
    m_center = sqrt(m_rCurv * m_rCurv - radius * radius);
    m_IOratio = m_muIn / m_muOut;

    lenslet->setRadius(radius);
    lenslet->setCurvatureRadius(m_rCurv);
    lenslet->setConvex(m_convex);

    // Cache lenslet radius
    m_lensletRadius = radius;
    
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
  aperture<ApertureArray>()->setWidth(width);
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setHeight(Real height)
{
  aperture<ApertureArray>()->setHeight(height);
  m_dirty  = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setCols(unsigned cols)
{
  aperture<ApertureArray>()->setCols(cols);
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setConvex(bool convex)
{
  m_convex = convex;
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setRows(unsigned rows)
{
  aperture<ApertureArray>()->setRows(rows);
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  Vec3 normal;
  uint64_t i;
  
  for (i = 0; i < count; ++i) {
    Vec3 origin = plane->toRelative(Vec3(beam.origins + 3 * i));
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));
    Real dt     = 0;

    if (aperture()->intercept(coord, normal, dt, origin)) {
      beam.lengths[i]       += dt;
      beam.cumOptLengths[i] += beam.n * dt;
      plane->fromRelative(coord).copyToArray(beam.destinations + 3 * i);
      beam.interceptDone(i);
      
      snell(
        Vec3(beam.directions + 3 * i), 
        plane->fromRelativeVec(normal),
        m_IOratio).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside lens
      beam.prune(i);
    }
  }

  beam.n = m_muIn;
}
