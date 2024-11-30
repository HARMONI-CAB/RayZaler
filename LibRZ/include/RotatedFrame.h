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

      virtual ~RotatedFrame() override = default;
  };
}

#endif // ROTATEDFRAME_H
