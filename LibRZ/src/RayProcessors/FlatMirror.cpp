#include <RayProcessors/FlatMirror.h>
#include <ReferenceFrame.h>

using namespace RZ;

std::string
FlatMirrorProcessor::name() const
{
  return "FlatMirror";
}

void
FlatMirrorProcessor::setRadius(Real R)
{
  m_radius = R;
}

void
FlatMirrorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Vec3 center = plane->getCenter();
  Vec3 tX     = plane->eX();
  Vec3 tY     = plane->eY();
  Vec3 normal = plane->eZ();
  Real Rsq    = m_radius * m_radius;
  
  for (i = 0; i < count; ++i) {
    // Check intercept
    Vec3 coord  = Vec3(beam.destinations + 3 * i) - plane->getCenter();
    Real coordX = coord * tX;
    Real coordY = coord * tY;

    if (coordX * coordX + coordY * coordY < Rsq) {
      // In mirror
      Vec3 direction(beam.directions + 3 * i);
      direction -= 2 * (direction * normal) * normal;
      direction.copyToArray(beam.directions + 3 * i);
    } else {
      // Outside mirror
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));
}

