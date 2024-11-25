#include <RayProcessors/ParabolicMirror.h>
#include <Surfaces/Parabolic.h>
#include <ReferenceFrame.h>

using namespace RZ;

ParabolicMirrorProcessor::ParabolicMirrorProcessor()
{
  setSurfaceShape(new ParabolicSurface(m_radius, m_flength));
}

std::string
ParabolicMirrorProcessor::name() const
{
  return "ParabolicMirror";
}

void
ParabolicMirrorProcessor::setRadius(Real R)
{
  m_radius = R;
  surfaceShape<ParabolicSurface>()->setRadius(R);
}

void
ParabolicMirrorProcessor::setFocalLength(Real f)
{
  m_flength = f;
  surfaceShape<ParabolicSurface>()->setFocalLength(f);
}

void
ParabolicMirrorProcessor::setCenterOffset(Real x, Real y)
{
  surfaceShape<ParabolicSurface>()->setCenterOffset(x, y);
}

void
ParabolicMirrorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;

  for (auto i = 0; i < count; ++i) {
    if (!beam.hasRay(i))
      continue;

    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));
    Vec3 origin = plane->toRelative(Vec3(beam.origins + 3 * i));
    Vec3 normal;
    Real dt;

    if (surfaceShape()->intercept(coord, normal, dt, origin)) {
      beam.lengths[i]       += dt;
      beam.cumOptLengths[i] += beam.n * dt;
      plane->fromRelative(coord).copyToArray(beam.destinations + 3 * i);
      beam.interceptDone(i);

      reflection(
        Vec3(beam.directions + 3 * i), 
        plane->fromRelativeVec(normal)).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside mirror
      beam.prune(i);
    }
  }
}
