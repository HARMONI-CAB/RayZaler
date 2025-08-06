//
//  Copyright (c) 2025 Gonzalo José Carracedo Carballal
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

#ifndef _ELEMENTS_WEDGEELEMENT_H
#define _ELEMENTS_WEDGEELEMENT_H

#include <Element.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;
  class RotatedFrame;

  class WedgeElement : public Element {
      TranslatedFrame *m_sides[6];
      RotatedFrame    *m_rotatedSides[6];
      GLWedge          m_wedge;
      
      Real m_cachedLength, m_cachedWidth, m_cachedHeights[2];
      void initSides();
      void recalculateTopSide();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      WedgeElement(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~WedgeElement() override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_ELEMENT(WedgeElement);
}

#endif // _ELEMENTS_WEDGEELEMENT_H
