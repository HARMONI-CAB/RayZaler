#include <RayProcessors.h>
#include <Singleton.h>
#include <ReferenceFrame.h>
#include <cstring>

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

std::string
InfiniteMirrorProcessor::name() const
{
  return "InfiniteMirror";
}

void
InfiniteMirrorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = 3 * beam.count;
  uint64_t i;
  Vec3 normal = plane->eZ();

  for (i = 0; i < count; ++i) {
    Vec3 direction(beam.directions + 3 * i);
    direction -= 2 * (direction * normal) * normal;
    direction.copyToArray(beam.directions + 3 * i);
  }

  memcpy(beam.origins, beam.destinations, count * sizeof(Real));
}

std::string
CircularMirrorProcessor::name() const
{
  return "CircularMirror";
}

void
CircularMirrorProcessor::setRadius(Real R)
{
  m_radius = R;
}

void
CircularMirrorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
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
      beam.lengths[i] = 10;
    } else {
      // Outside mirror
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));
}

std::string
SphericalMirrorProcessor::name() const
{
  return "ConcaveMirror";
}

void
SphericalMirrorProcessor::setRadius(Real R)
{
  m_radius = R;
}

//
// This is basically a line-sphere intersection. Note that, the center of the
// sphere is located at:
//    C = M.center + f * M.eZ()
//
// The discriminant is:
//    Delta = (u * (O - C))^2 - (||O - C||^2 - R^2)
//
// With R^2 = f^2, O the origin of the ray, and u its direction
//
// If Delta >= 0, an intersection exists at a distance
//   t = -(u * (O - C)) + sqrt(Delta)
//  
//  
void
SphericalMirrorProcessor::setFocalLength(Real f)
{
  m_flength = f;
}


void
SphericalMirrorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Vec3 center  = plane->getCenter();
  Vec3 tX      = plane->eX();
  Vec3 tY      = plane->eY();
  Vec3 normal  = plane->eZ();
  Real Rcurv   = 2 * m_flength;
  Vec3 Csphere = center + Rcurv * normal;
  Real Rsq     = m_radius * m_radius;
  Real t;

  for (i = 0; i < count; ++i) {
    Vec3 coord  = Vec3(beam.destinations + 3 * i) - plane->getCenter();
    Real coordX = coord * tX;
    Real coordY = coord * tY;

    if (coordX * coordX + coordY * coordY < Rsq) {
      // In mirror, calculate intersection
      Vec3 O  = Vec3(beam.origins + 3 * i);
      Vec3 OC = O - Csphere;
      Vec3 u(beam.directions + 3 * i);
      Real uOC   = u * OC;
      Real Delta = uOC * uOC - OC * OC + Rcurv * Rcurv;
      
      if (Delta < 0) {
        beam.prune(i);
      } else {
        t = -uOC + sqrt(Delta);
        Vec3 destination(O + t * u);
        Vec3 normal = (Csphere - destination).normalized();

        destination.copyToArray(beam.destinations + 3 * i);

        u -= 2 * (u * normal) * normal;
        u.copyToArray(beam.directions + 3 * i);
        beam.lengths[i] = 10;
      }
    } else {
      // Outside mirror
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));
}


void
RZ::registerRayProcessors()
{
  Singleton::instance()->registerRayTransferProcessor(new PassThroughProcessor);
  Singleton::instance()->registerRayTransferProcessor(new InfiniteMirrorProcessor);
}
