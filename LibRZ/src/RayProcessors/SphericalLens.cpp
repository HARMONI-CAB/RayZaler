#include <RayProcessors/SphericalLens.h>
#include <ReferenceFrame.h>

using namespace RZ;

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
      }
    } else {
      // Outside lens
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));

  // Yes, we specify the material these rays had to traverse
  beam.n = m_muIn;
}
