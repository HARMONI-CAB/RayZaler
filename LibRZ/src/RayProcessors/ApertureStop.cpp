#include <RayProcessors/ApertureStop.h>
#include <Apertures/Circular.h>

#include <ReferenceFrame.h>

using namespace RZ;

ApertureStopProcessor::ApertureStopProcessor()
{
  defineAperture(new CircularAperture(m_radius));
}

std::string
ApertureStopProcessor::name() const
{
  return "ApertureStop";
}

void
ApertureStopProcessor::setRadius(Real R)
{
  aperture<CircularAperture>()->setRadius(R);
  
  m_radius = R;
}

void
ApertureStopProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;

  for (i = 0; i < count; ++i) {
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));

    if (!aperture()->intercept(coord))
      beam.prune(i);
    else
      beam.interceptDone(i);
  }
}
