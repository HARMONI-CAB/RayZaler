#include <RayProcessors/PassThrough.h>

using namespace RZ;

std::string
PassThroughProcessor::name() const
{
  return "PassThroughProcessor";
}

void
PassThroughProcessor::process(RayBeam &beam, const ReferenceFrame *) const
{
  // No processing
}

