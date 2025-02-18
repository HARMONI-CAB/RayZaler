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

#ifndef _RAY_PROCESSORS_CONIC_MIRROR_H
#define _RAY_PROCESSORS_CONIC_MIRROR_H

#include <RayTracingEngine.h>

namespace RZ {
  class ReferenceFrame;

  class ConicMirrorBoundary : public MediumBoundary {
      bool m_convex  = false;

    public:
      ConicMirrorBoundary();
      virtual ~ConicMirrorBoundary() = default;
      void setRadius(Real);
      void setCenterOffset(Real, Real);

      void setCurvatureRadius(Real);
      void setConicConstant(Real);
      void setHoleRadius(Real);
      void setConvex(bool);

      virtual std::string name() const;
  };
}

#endif // _RAY_PROCESSORS_CONIC_MIRROR_H
