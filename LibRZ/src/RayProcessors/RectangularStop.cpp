#include <RayProcessors/RectangularStop.h>
#include <ReferenceFrame.h>
#include <Apertures/Rectangular.h>

using namespace RZ;

RectangularStopProcessor::RectangularStopProcessor()
{
  defineAperture(new RectangularAperture());

  aperture<RectangularAperture>()->setHeight(m_height);
  aperture<RectangularAperture>()->setHeight(m_width);
}

std::string
RectangularStopProcessor::name() const
{
  return "RectangularStop";
}

void
RectangularStopProcessor::setWidth(Real width)
{
  m_width = width;
  aperture<RectangularAperture>()->setHeight(m_width);
}

void
RectangularStopProcessor::setHeight(Real height)
{
  m_height = height;
  aperture<RectangularAperture>()->setHeight(m_height);
}

void
RectangularStopProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Vec3 center  = plane->getCenter();
  Vec3 tX      = plane->eX();
  Vec3 tY      = plane->eY();
  Real halfW   = .5 * m_width;
  Real halfH   = .5 * m_height;

  for (i = 0; i < count; ++i) {
    Vec3 coord  = Vec3(beam.destinations + 3 * i) - plane->getCenter();
    Real coordX = coord * tX;
    Real coordY = coord * tY;

    if (fabs(coordX) >= halfW || fabs(coordY) >= halfH)
      beam.prune(i);
  }
}
