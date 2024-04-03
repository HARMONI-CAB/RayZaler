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
  bool convex = f > 0;
  
  aperture<SphericalAperture>()->setConvex(convex);
  aperture<SphericalAperture>()->setCurvatureRadius(2 * f);
}

void
SphericalMirrorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Real dt;

  for (i = 0; i < count; ++i) {
    if (!beam.hasRay(i))
      continue;
    
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));
    Vec3 orig   = plane->toRelative(Vec3(beam.origins + 3 * i));
    Vec3 normal;

    if (aperture()->intercept(coord, normal, dt, orig)) {
      beam.lengths[i]       += dt;
      beam.cumOptLengths[i] += beam.n * dt;
      
      plane->fromRelative(coord).copyToArray(beam.destinations + 3 * i);
      reflection(
        Vec3(beam.directions + 3 * i), 
        plane->fromRelativeVec(normal)).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside lens
      beam.prune(i);
    }
  }
}
