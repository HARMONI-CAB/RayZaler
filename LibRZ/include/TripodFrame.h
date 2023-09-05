#ifndef TRIPODFRAME_H
#define TRIPODFRAME_H

#include <ReferenceFrame.h>

namespace RZ {
  class TripodFrame : public ReferenceFrame {

      Vec3    m_center;
      Matrix3 m_R;

      // Parameters
      Real m_legs[3]    = {2e-2, 2e-2, 2e-2};
      Real m_ta_angle   = 70.; // Deg
      Real m_ta_radius  = 42e-3;

      // Calculated
      Vec3 m_p1;
      Vec3 m_p2;
      Vec3 m_p3;

      int m_centerIndex = -1;

      void recalculateData();

    protected:
      void recalculateFrame();

    public:
      TripodFrame(
        std::string const &name,
        ReferenceFrame *parent);

      void setLeg(int, Real);
      void setRadius(Real);
      void setAngle(Real);
  };
}

#endif // TRIPODFRAME_H
