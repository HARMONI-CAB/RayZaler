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
      beam.lengths[i] = 10;
    } else {
      // Outside mirror
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));
}

//////////////////////////// SphericalMirrorProcessor //////////////////////////
std::string
SphericalMirrorProcessor::name() const
{
  return "SphericalMirror";
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
  Real Rsq     = m_radius * m_radius;
  Real d       = Rcurv - sqrt(Rcurv * Rcurv - Rsq);
  Vec3 Csphere = center + (Rcurv - d) * normal;
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

//////////////////////////// ParabolicMirrorProcessor //////////////////////////
std::string
ParabolicMirrorProcessor::name() const
{
  return "ParabolicMirror";
}

void
ParabolicMirrorProcessor::setRadius(Real R)
{
  m_radius = R;
}

void
ParabolicMirrorProcessor::setFocalLength(Real f)
{
  m_flength = f;
}

void
ParabolicMirrorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  Matrix3 M      = plane->getOrientation();
  Matrix3 Mt     = M.t();
  Real Rsq       = m_radius * m_radius;
  Real k         =  .25 / m_flength;
  Real h         = k * Rsq;

  for (auto i = 0; i < count; ++i) {
    Vec3 intercept  = Mt * (Vec3(beam.destinations + 3 * i) - plane->getCenter());

    if (intercept.x * intercept.x + intercept.y * intercept.y < Rsq) {
      // In mirror, calculate intersection
      Vec3 u      = Vec3(beam.directions + 3 * i);
      Vec3 ut     = Mt * u; // Direction in the reference plane coordinates
      Vec3 O      = Vec3(beam.origins + 3 * i);
      Vec3 Ot     = Mt * (O - plane->getCenter());

      Real a = ut.x;
      Real b = ut.y;
      Real c = ut.z;

      Real eqA   = a * a + b * b;
      Real eqB   = 2 * (a * Ot.x + b * Ot.y - 2 * m_flength);
      Real eqC   = Ot.x * Ot.x + Ot.y * Ot.y - 4 * m_flength * Ot.z - Rsq;
      Real Delta = eqB * eqB - 4 * eqA * eqC;
      Real t;

      if (eqA <= std::numeric_limits<Real>::epsilon()) {
        t = -eqC / eqB;
      } else if (Delta >= 0) {
        t = .5 * (-eqB + sqrt(Delta)) / eqA;
      } else {
        beam.prune(i);
        continue;
      }

      Vec3 destination(O + t * u);
      Vec3 destt(Ot + t * ut);
      Vec3 normal = M * Vec3(-2 * k * destt.x, -2 * k * destt.y, 1).normalized();

      destination.copyToArray(beam.destinations + 3 * i);

      u -= 2 * (u * normal) * normal;
      u.copyToArray(beam.directions + 3 * i);
      beam.lengths[i] = 10;
    } else {
      // Outside mirror
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));
}

////////////////////////// SphericalLensProcessor //////////////////////////////
std::string
SphericalLensProcessor::name() const
{
  return "SphericalLens";
}

void
SphericalLensProcessor::recalcCurvCenter()
{
  m_center = sqrt(m_rCurv * m_rCurv - m_radius * m_radius);
}

void
SphericalLensProcessor::setRadius(Real R)
{
  m_radius = R;
  recalcCurvCenter();
}

void
SphericalLensProcessor::setCurvatureRadius(Real R)
{
  m_rCurv = R;
  recalcCurvCenter();
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
SphericalLensProcessor::setRefractiveIndex(Real in, Real out)
{
  m_muIn  = in;
  m_muOut = out;
  m_IOratio = in / out;
}

void
SphericalLensProcessor::setConvex(bool convex)
{
  m_convex = convex;
}

//
// Spherical lens processors describe the intersection of a ray on something
// constructed like this:
//
//  (A)              (B)
//   |                |
//   |\              /|
//   | |     or     | |
//   | |    this    | |
//   |/              \|
//   |                |
//  InS              Ins
//
// -----> Direction of propagation --->
//
//   Lens bumps a distance d from the intercept surface, which means
//   that the curvature center is a distance Rc - d to the left. In the
//   edge of the lens, the curvature center is exactly at a distance Rc, and
//   at a distance R to the vertical distance to the center of the lens. This
//   means that:
//   
//   R = Rc * sin(alpha)
//
//   On the other hand, cos(alpha) = (Rc - d) / Rc = 1 - d / Rc
//
//   Since cos^2 + sin^2 = 1, then:
//
//   R^2 = Rc^2 (1 - sin^2) = Rc^2 * (1 - (1 - d/Rc)^2) =
//       = Rc^2 (1 - 1 - d^2/Rc^2 + 2d/Rc)
//       = Rc^2 (2d/Rc - d^2 / Rc^2) = 
//       = 2dRc - d^2
//  
//   We have to solve for d:
//
//   d^2 - 2Rc d + R^2 = 0
//
//   There are two solutions for d:
//
//   d1 = Rc + sqrt(Rc^2 - R^2)
//   d2 = Rc - sqrt(Rc^2 - R^2)
//
//   And conversely, for the center of curvature, in the propagation axis:
//
//   C1 = - sqrt(Rc^2 - R^2)
//   C2 = + sqrt(Rc^2 - R^2)
//
//   In case A, the center of curvature is C1. In case B, it is C2
//
//   Now, when we compute the intersection, we are going to have 2 origin-relative
//   solutions for t. t1 < t2. t1 is the convex one, and t2 is the concave one.
//  

void
SphericalLensProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Vec3 center  = plane->getCenter();
  Vec3 tX      = plane->eX();
  Vec3 tY      = plane->eY();
  Vec3 normal  = plane->eZ();
  Real Rcurv   = m_rCurv;
  Real sign    = m_convex ? +1 : -1;
  Vec3 Csphere = center + sign * m_center * normal;
  Real Rsq     = m_radius * m_radius;
  Real t;

  for (i = 0; i < count; ++i) {
    Vec3 coord  = Vec3(beam.destinations + 3 * i) - plane->getCenter();
    Real coordX = coord * tX;
    Real coordY = coord * tY;

    if (coordX * coordX + coordY * coordY < Rsq) {
      // In lens, calculate intersection
      Vec3 O  = Vec3(beam.origins + 3 * i);
      Vec3 OC = O - Csphere;
      Vec3 u(beam.directions + 3 * i);
      Real uOC   = u * OC;
      Real Delta = uOC * uOC - OC * OC + Rcurv * Rcurv;
      bool inverted = false;

      if (Delta < 0) {
        beam.prune(i);
      } else {
        Real sqDelta = sqrt(Delta);
        t = m_convex ? -uOC - sqDelta : -uOC + sqDelta;
        
        // Calculation of lens intersection
        Vec3 destination(O + t * u);
        Vec3 normal = (destination - Csphere).normalized();
        
        if (!m_convex)
          normal = -normal;
        Vec3 nXu    = m_IOratio * normal.cross(u);

        destination.copyToArray(beam.destinations + 3 * i);

        // Some Snell
        u  = -normal.cross(nXu) - normal * sqrt(1 - nXu * nXu);
        u.copyToArray(beam.directions + 3 * i);
        beam.lengths[i] = 10;
      }
    } else {
      // Outside lens
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));
}

///////////////////////////// Aperture Stop ////////////////////////////////////
std::string
ApertureStopProcessor::name() const
{
  return "ApertureStop";
}

void
ApertureStopProcessor::setRadius(Real R)
{
  m_radius = R;
}

void
ApertureStopProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
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

    if (coordX * coordX + coordY * coordY >= Rsq)
      beam.prune(i);
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));
}

///////////////////////////// Circ. Obstruction ////////////////////////////////
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

void
RZ::registerRayProcessors()
{
  Singleton::instance()->registerRayTransferProcessor(new PassThroughProcessor);
  Singleton::instance()->registerRayTransferProcessor(new InfiniteMirrorProcessor);
}
