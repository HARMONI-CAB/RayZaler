#include <RayProcessors/SphericalMirror.h>
#include <Surfaces/Spherical.h>
#include <ReferenceFrame.h>

using namespace RZ;

SphericalMirrorProcessor::SphericalMirrorProcessor()
{
  setSurfaceShape(new SphericalSurface(m_radius, 2 * m_flength));
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
  surfaceShape<SphericalSurface>()->setRadius(R);
}

void
SphericalMirrorProcessor::setCenterOffset(Real x, Real y)
{
  m_x0 = x;
  m_y0 = y;

  surfaceShape<SphericalSurface>()->setCenterOffset(x, y);
}

void
SphericalMirrorProcessor::setFocalLength(Real f)
{
  bool convex = f > 0;
  
  surfaceShape<SphericalSurface>()->setConvex(convex);
  surfaceShape<SphericalSurface>()->setCurvatureRadius(2 * f);
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

    if (surfaceShape()->intercept(coord, normal, dt, orig)) {
      beam.lengths[i]       += dt;
      beam.cumOptLengths[i] += beam.n * dt;

      plane->fromRelative(coord).copyToArray(beam.destinations + 3 * i);
      beam.interceptDone(i);
      
      reflection(
        Vec3(beam.directions + 3 * i), 
        plane->fromRelativeVec(normal)).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside lens
      beam.prune(i);
    }
  }
}
