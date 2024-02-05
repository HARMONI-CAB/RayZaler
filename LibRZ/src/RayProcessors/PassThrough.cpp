#include <RayProcessors/PassThrough.h>

using namespace RZ;

std::string
PassThroughProcessor::name() const
{
  return "PassThrough";
}

void
PassThroughProcessor::process(RayBeam &beam, const ReferenceFrame *) const
{
  uint64_t count = 3 * beam.count;
  uint64_t i;

  memcpy(beam.origins, beam.destinations, count * sizeof(Real));
}

