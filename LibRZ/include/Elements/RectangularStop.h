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

#ifndef _RECTANGULAR_STOP_H
#define _RECTANGULAR_STOP_H

#include <OpticalElement.h>
#include <MediumBoundaries/RectangularStop.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class RectangularStop : public OpticalElement {
      RectangularStopBoundary *m_boundary;
      GLRectangle m_vRect, m_hRect;
      TranslatedFrame *m_stopSurface = nullptr;
      
      Real m_width  = 3e-2;
      Real m_height = 3e-2;

      Real m_borderWidth  = 4e-2;
      Real m_borderHeight = 4e-2;

      Real m_hShift, m_vShift;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      RectangularStop(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~RectangularStop() override;

      virtual void nativeMaterialOpenGL(std::string const &role) override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_OPTICAL_ELEMENT(RectangularStop);
}

#endif // _RECTANGULAR_STOP_H
