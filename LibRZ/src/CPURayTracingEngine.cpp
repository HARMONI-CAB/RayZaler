#include <CPURayTracingEngine.h>

using namespace RZ;

CPURayTracingEngine::CPURayTracingEngine() : RayTracingEngine()
{

}

void
CPURayTracingEngine::cast(Point3 const &center,  Vec3 const &normal)
{
  RayBeam *beam = this->beam();
  Real numDot, demDot, t;
  uint64_t i, N = 3 * beam->count, r = 0;
  uint64_t p = 0;

  numDot = demDot = 0;
  for (i = 0; i < N; ++i) {
    beam->destinations[i] = center.coords[p] - beam->origins[i];

    numDot += normal.coords[p] * beam->destinations[i];
    demDot += normal.coords[p] * beam->directions[i];

    if (++p == 3) {
      p = 0;
      t = numDot / demDot;
      numDot = demDot = 0;
      
      beam->destinations[i - 2] = beam->origins[i - 2] + t * beam->directions[i - 2];
      beam->destinations[i - 1] = beam->origins[i - 1] + t * beam->directions[i - 1];
      beam->destinations[i - 0] = beam->origins[i - 0] + t * beam->directions[i - 0];
      beam->lengths[r++] = t;
    }
  }
}
