#include <RayProcessors/CircularWindow.h>
#include <Surfaces/Circular.h>
#include <ReferenceFrame.h>

using namespace RZ;

CircularWindowProcessor::CircularWindowProcessor()
{
  setSurfaceShape(new CircularFlatSurface(m_radius));
}

std::string
CircularWindowProcessor::name() const
{
  return "CircularWindowProcessor";
}

void
CircularWindowProcessor::setRadius(Real R)
{
  m_radius = R;
  surfaceShape<CircularFlatSurface>()->setRadius(R);
}

void
CircularWindowProcessor::setRefractiveIndex(Real in, Real out)
{
  m_muIn  = in;
  m_muOut = out;
  m_IOratio = in / out;
}

void
CircularWindowProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;

  for (i = 0; i < count; ++i) {
    if (!beam.hasRay(i))
      continue;
    
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));
    Vec3 orig   = plane->toRelative(Vec3(beam.origins + 3 * i));
    Vec3 normal = Vec3::eZ();

    if (surfaceShape()->intercept(coord)) {
      beam.interceptDone(i);
      
      snell(
        Vec3(beam.directions + 3 * i), 
        plane->fromRelativeVec(normal),
        m_IOratio).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside window
      beam.prune(i);
    }
  }

  // Yes, we specify the material these rays had to traverse
  beam.n = m_muOut;
}
