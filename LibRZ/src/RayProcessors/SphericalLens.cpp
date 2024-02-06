#include <RayProcessors/SphericalLens.h>
#include <Apertures/Spherical.h>
#include <ReferenceFrame.h>

using namespace RZ;

SphericalLensProcessor::SphericalLensProcessor()
{
  defineAperture(new SphericalAperture(m_radius, m_rCurv));
}

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
  aperture<SphericalAperture>()->setRadius(R);
  recalcCurvCenter();
}

void
SphericalLensProcessor::setCurvatureRadius(Real R)
{
  m_rCurv = R;
  aperture<SphericalAperture>()->setCurvatureRadius(R);
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
  aperture<SphericalAperture>()->setConvex(convex);
}

void
SphericalLensProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Real offZ    = aperture<SphericalAperture>()->centerOffset();
  Vec3 Csphere = plane->fromRelative(Vec3(0, 0, offZ));

  for (i = 0; i < count; ++i) {
    if (!beam.hasRay(i))
      continue;
    
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));
    Vec3 orig   = plane->toRelative(Vec3(beam.origins + 3 * i));
    Vec3 normal;

    if (aperture()->intercept(coord, normal, orig)) {
      plane->fromRelative(coord).copyToArray(beam.destinations + 3 * i);
      snell(
        Vec3(beam.directions + 3 * i), 
        plane->fromRelativeVec(normal),
        m_IOratio).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside lens
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));

  // Yes, we specify the material these rays had to traverse
  beam.n = m_muIn;
}

#if 0
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
#endif