#include <TripodFrame.h>

using namespace RZ;

void
TripodFrame::recalculateData()
{
  Vec3 v1, v2, nt, k;
  Matrix3 R;
  Real nproj;

  m_p1 = Vec3(
      + m_ta_radius * sin(deg2rad(.5 * m_ta_angle)),
      + m_ta_radius * cos(deg2rad(.5 * m_ta_angle)),
      m_legs[0]);

  m_p2 = Vec3(
      - m_ta_radius * sin(deg2rad(.5 * m_ta_angle)),
      + m_ta_radius * cos(deg2rad(.5 * m_ta_angle)),
      m_legs[1]);
  
  m_p3 = Vec3(
      0,
      -m_ta_radius,
      m_legs[2]);

  // In order to calculate this new system, we will proceed as follows:
  // 1. Evaluate the normal of the tilted plane n_t
  // 2. Cross product it by the mirror normal n_m. Now we have a rotation
  //    axis k.
  // 3. Now, the dot product of n_t · n_m is cos of the rotation, while
  //    the norm of the cross product of n_t x n_m is the sin of the rotation. 
  //    We can turn this into a rotation matrix simply by adding
  //
  //    R = (cos theta) I + K
  //
  //    Where K is the equivalent matrix of the cross product by l.

  v1 = m_p1 - m_p3;
  v2 = m_p2 - m_p3;

  nt    = v1.cross(v2).normalized();
  nproj = nt.z;

  k = -Vec3::eZ().cross(nt);
  R = nproj * Matrix3::eye() + Matrix3::crossMatrix(k);

  // We now need to calculate the circumcenter of the new triangle. This
  // is what the new center of our system will be, with respect to its
  // zero position

  Real m2_1 = v1 * v1;
  Real m2_2 = v2 * v2;
  Vec3 C    = v1.cross(v2);
  Real Cm2  = C * C;
  Vec3 dir1 = m2_1 * C.cross(v1);
  Vec3 dir2 = m2_2 * v2.cross(C);

  m_center  = m_p3 + (dir1 + dir2) / (2 * Cm2);
  m_R       = R;
}

void
TripodFrame::setLeg(int index, Real val)
{
  if (index >= 0 && index < 3) {
    m_legs[index] = val;
    recalculateData();
  }
}

void
TripodFrame::setRadius(Real radius)
{
  m_ta_radius = radius;
  recalculateData();
}

void
TripodFrame::setAngle(Real angle)
{
  m_ta_angle = angle;
  recalculateData();
}

TripodFrame::TripodFrame(
  std::string const &name,
  ReferenceFrame *parent) : ReferenceFrame(name, parent)
{
  m_typeId = RZ_REF_FRAME_TRIPOD_ID;
  
  recalculateData();

  // Add displacement vector 
  m_centerIndex = parent->addAxis(name + ".center", m_center);
}

void
TripodFrame::recalculateFrame()
{
  Matrix3 absOrientation = m_R * parent()->getOrientation();
  setOrientation(absOrientation);
  setCenter(parent()->getCenter() + absOrientation.t() * m_center);
}
