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

#ifndef _RODELEMENT_H
#define _RODELEMENT_H

#include <Element.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;
  class RotatedFrame;
  
  class RodElement : public Element {
      TranslatedFrame *m_sides[3];        // Top, middle, bottom
      RotatedFrame    *m_rotatedSides[3]; // Top, middle, bottom

      Real m_cachedLength = 5e-2;
      Real m_cachedDiameter = 3e-3;

      GLCappedCylinder m_cylinder;

      void recalcBoundingBox();
      void initSides();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      RodElement(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~RodElement() override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_ELEMENT(RodElement);
}

#endif // _RODELEMENT_H
