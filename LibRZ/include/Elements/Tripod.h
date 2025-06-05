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

#ifndef _TRIPOD_H
#define _TRIPOD_H

#include <Element.h>
#include <GLHelpers.h>
#include <TripodFrame.h>

namespace RZ {
  class TranslatedFrame;
  class RotatedFrame;

  class Tripod : public Element {
      GLCappedCylinder m_glLegs[3];
      TripodFrame     *m_surface;

      Real m_legDiameter = 6e-3;
      Real m_leg1        = 2e-2;
      Real m_leg2        = 2e-2;
      Real m_leg3        = 2e-2;
      Real m_ta_angle    = 70.; // Deg
      Real m_ta_radius   = 42e-3;

      // Calculated
      Vec3 m_p[3];

      void recalcLegs();
      void initTripod();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      Tripod(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~Tripod() override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_ELEMENT(Tripod);
}

#endif // _TRIPOD_H
