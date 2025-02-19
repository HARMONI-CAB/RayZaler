#include <RayProcessors/InfiniteMirror.h>
#include <ReferenceFrame.h>

using namespace RZ;

std::string
InfiniteMirrorProcessor::name() const
{
  return "InfiniteMirrorProcessor";
}

void
InfiniteMirrorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = 3 * beam.count;
  uint64_t i;
  Vec3 normal = plane->eZ();

  for (i = 0; i < count; ++i) {
    beam.interceptDone(i);
    reflection(
        Vec3(beam.directions + 3 * i), 
        normal).copyToArray(beam.directions + 3 * i);
  }
}
