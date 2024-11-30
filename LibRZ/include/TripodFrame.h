//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

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
      virtual ~TripodFrame() override = default;
      
      void setLeg(int, Real);
      void setRadius(Real);
      void setAngle(Real);
  };
}

#endif // TRIPODFRAME_H
