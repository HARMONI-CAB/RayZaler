#include <RayProcessors/FlatMirror.h>
#include <Apertures/Circular.h>
#include <ReferenceFrame.h>

using namespace RZ;

FlatMirrorProcessor::FlatMirrorProcessor()
{
  defineAperture(new CircularAperture(m_radius));
}

std::string
FlatMirrorProcessor::name() const
{
  return "FlatMirror";
}

void
FlatMirrorProcessor::setRadius(Real R)
{
  aperture<CircularAperture>()->setRadius(R);

  m_radius = R;
}

void
FlatMirrorProcessor::setEccentricity(Real ecc)
{
  aperture<CircularAperture>()->setEccentricity(ecc);

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

    if (aperture()->intercept(coord)) {
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
