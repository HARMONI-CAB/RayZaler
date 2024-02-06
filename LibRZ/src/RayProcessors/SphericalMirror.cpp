#include <RayProcessors/SphericalMirror.h>
#include <Apertures/Spherical.h>
#include <ReferenceFrame.h>

using namespace RZ;

SphericalMirrorProcessor::SphericalMirrorProcessor()
{
  defineAperture(new SphericalAperture(m_radius, 2 * m_flength));
}

std::string
SphericalMirrorProcessor::name() const
{
  return "SphericalMirror";
}

void
SphericalMirrorProcessor::setRadius(Real R)
{
  m_radius = R;
  aperture<SphericalAperture>()->setRadius(R);
}

void
SphericalMirrorProcessor::setFocalLength(Real f)
{
  m_flength = f;
  aperture<SphericalAperture>()->setCurvatureRadius(2 * f);
}

void
SphericalMirrorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;

  for (i = 0; i < count; ++i) {
    if (!beam.hasRay(i))
      continue;
    
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));
    Vec3 orig   = plane->toRelative(Vec3(beam.origins + 3 * i));
    Vec3 normal;

    if (aperture()->intercept(coord, normal, orig)) {
      plane->fromRelative(coord).copyToArray(beam.destinations + 3 * i);
      reflection(
        Vec3(beam.directions + 3 * i), 
        plane->fromRelativeVec(normal)).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside lens
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));
}
