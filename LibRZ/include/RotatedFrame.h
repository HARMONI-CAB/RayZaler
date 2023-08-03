#ifndef ROTATEDFRAME_H
#define ROTATEDFRAME_H

#include <ReferenceFrame.h>

namespace RZ {
  class RotatedFrame : public ReferenceFrame {
      Vec3    m_currAxis;
      Real    m_currAngle;
      int     m_axisIndex;
      
      Matrix3 m_rotMatrix;

    protected:
      void recalculateFrame();

    public:
      void setRotation(Vec3 const &axis, Real angle);
      void setAxisX(Real);
      void setAxisY(Real);
      void setAxisZ(Real);
      void setAngle(Real);

      RotatedFrame(
        std::string const &name,
        ReferenceFrame *parent,
        Vec3 const &axis,
        Real angle);
  };
}

#endif // ROTATEDFRAME_H
