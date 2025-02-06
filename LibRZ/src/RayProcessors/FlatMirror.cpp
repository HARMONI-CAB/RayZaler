#include <RayProcessors/FlatMirror.h>
#include <Surfaces/Circular.h>
#include <ReferenceFrame.h>

using namespace RZ;

FlatMirrorProcessor::FlatMirrorProcessor()
{
  setSurfaceShape(new CircularFlatSurface(m_radius));
}

std::string
FlatMirrorProcessor::name() const
{
  return "FlatMirrorProcessor";
}

void
FlatMirrorProcessor::setRadius(Real R)
{
  surfaceShape<CircularFlatSurface>()->setRadius(R);

  m_radius = R;
}

void
FlatMirrorProcessor::setEccentricity(Real ecc)
{
  surfaceShape<CircularFlatSurface>()->setEccentricity(ecc);

  m_ecc = ecc;
}

void
FlatMirrorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Vec3 normal = plane->eZ();
  
  for (i = 0; i < count; ++i) {
    if (!beam.hasRay(i))
      continue;
    
    // Check intercept
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));

    if (surfaceShape()->intercept(coord)) {
      beam.interceptDone(i);
      reflection(
        Vec3(beam.directions + 3 * i), 
        normal).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside mirror
      beam.prune(i);
    }
  }
}
