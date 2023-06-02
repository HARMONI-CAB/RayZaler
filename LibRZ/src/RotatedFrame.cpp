#include <RotatedFrame.h>

using namespace RZ;

void
RotatedFrame::setAxisX(Real val)
{
  m_currAxis.x = val;
  
  parent()->getAxis(m_axisIndex)->x = val;
  m_rotMatrix = Matrix3::rot(m_currAxis.normalized(), m_currAngle);
}

void
RotatedFrame::setAxisY(Real val)
{
  m_currAxis.y = val;

  parent()->getAxis(m_axisIndex)->y = val;
  m_rotMatrix = Matrix3::rot(m_currAxis.normalized(), m_currAngle);
}

void
RotatedFrame::setAxisZ(Real val)
{
  m_currAxis.z = val;

  parent()->getAxis(m_axisIndex)->z = val;
  m_rotMatrix = Matrix3::rot(m_currAxis.normalized(), m_currAngle);
}

void
RotatedFrame::setAngle(Real val)
{
  m_currAngle = val;
  m_rotMatrix = Matrix3::rot(m_currAxis.normalized(), m_currAngle);
}

void
RotatedFrame::setRotation(Vec3 const &axis, Real angle)
{
  m_currAxis = axis;
  m_currAngle = angle;
  
  m_rotMatrix = Matrix3::rot(m_currAxis.normalized(), m_currAngle);

  // Set displacement vector
  *parent()->getAxis(m_axisIndex) = m_currAxis;
}

RotatedFrame::RotatedFrame(
  std::string const &name,
  ReferenceFrame *parent,
  Vec3 const &axis,
  Real angle) : ReferenceFrame(name, parent)
{
  m_typeId    = RZ_REF_FRAME_ROTATION_ID;
  m_rotMatrix = Matrix3::rot(axis.normalized(), angle);

  m_currAxis  = axis;
  m_currAngle = angle;

  // Add displacement vector 
  m_axisIndex = parent->addAxis(name + ".axis", axis);
}

void
RotatedFrame::recalculateFrame()
{
  setOrientation(m_rotMatrix * parent()->getOrientation());
  setCenter(parent()->getCenter());
}
