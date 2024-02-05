#include <RayProcessors/ParabolicMirror.h>
#include <ReferenceFrame.h>

using namespace RZ;

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
  Matrix3 M      = plane->getOrientation().t();
  Matrix3 Mt     = M.t();
  Real Rsq       = m_radius * m_radius;
  Real k         =  .25 / m_flength;

  for (auto i = 0; i < count; ++i) {
    Vec3 intercept  = Mt * (Vec3(beam.destinations + 3 * i) - plane->getCenter());

    if (intercept.x * intercept.x + intercept.y * intercept.y < Rsq) {
      // In mirror, calculate intersection
      Vec3 u      = Vec3(beam.directions + 3 * i);
      Vec3 ut     = Mt * u; // Direction in the reference plane coordinates
      Vec3 O      = Vec3(beam.origins + 3 * i);
      Vec3 Ot     = Mt * (O - plane->getCenter());

      Real x = Ot.x;
      Real y = Ot.y;
      Real z = Ot.z;

      if (z < 0)
        beam.prune(i);

      Real a = ut.x;
      Real b = ut.y;
      Real c = ut.z;

      // Now, we solve a quadratic equation of the form:
      //
      // At^2 + Bt + C = 0

      // Coefficient A: just the projection of the ray on the XY plane
      Real A     = a * a + b * b;

      // Equation B: this is just the rect equation
      Real B     = 2 * (a * x + b * y + 2 * m_flength * c);
      Real C     = x * x + y * y - Rsq + 4 * m_flength * z;
      Real Delta = B * B - 4 * A * C;
      Real t;

      if (A <= std::numeric_limits<Real>::epsilon()) {
        t = -C / B;
      } else if (Delta >= 0) {
        t = .5 * (-B + sqrt(Delta)) / A;
      } else {
        beam.prune(i);
        continue;
      }

      
      Vec3 destination(O + t * u);
      Vec3 destt(Ot + t * ut);
      Vec3 normal = M * Vec3(2 * k * destt.x, 2 * k * destt.y, 1).normalized();

      destination.copyToArray(beam.destinations + 3 * i);

      u -= 2 * (u * normal) * normal;
      u.copyToArray(beam.directions + 3 * i);
      
      // Single reflection, no length increment
    } else {
      // Outside mirror
      beam.prune(i);
    }
  }

  memcpy(beam.origins, beam.destinations, 3 * count * sizeof(Real));
}
