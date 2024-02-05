#include <RayProcessors/Obstruction.h>
#include <ReferenceFrame.h>

using namespace RZ;

std::string
ObstructionProcessor::name() const
{
  return "Obstruction";
}

void
ObstructionProcessor::setRadius(Real R)
{
  m_radius = R;
}

void
ObstructionProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Vec3 center  = plane->getCenter();
  Vec3 tX      = plane->eX();
  Vec3 tY      = plane->eY();
  Real Rsq     = m_radius * m_radius;

  for (i = 0; i < count; ++i) {
    Vec3 coord  = Vec3(beam.destinations + 3 * i) - plane->getCenter();
    Real coordX = coord * tX;
    Real coordY = coord * tY;

    if (coordX * coordX + coordY * coordY <= Rsq)
      beam.prune(i);
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));
}
