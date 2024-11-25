#include <RayProcessors/ApertureStop.h>
#include <Surfaces/Circular.h>

#include <ReferenceFrame.h>

using namespace RZ;

ApertureStopProcessor::ApertureStopProcessor()
{
  setReversible(true);
  setSurfaceShape(new CircularFlatSurface(m_radius));
}

std::string
ApertureStopProcessor::name() const
{
  return "ApertureStop";
}

void
ApertureStopProcessor::setRadius(Real R)
{
  surfaceShape<CircularFlatSurface>()->setRadius(R);
  
  m_radius = R;
}

void
ApertureStopProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;

  for (i = 0; i < count; ++i) {
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));

    if (!surfaceShape()->intercept(coord))
      beam.prune(i);
    else
      beam.interceptDone(i);
  }
}
