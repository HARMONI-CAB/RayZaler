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

#ifndef _ELEMENTS_BABINET_H
#define _ELEMENTS_BABINET_H

#include <OpticalElement.h>
#include <GLHelpers.h>
#include <MediumBoundaries/RectangularWindow.h>
#include <EMInterface.h>

namespace RZ {
  class TranslatedFrame;
  class RotatedFrame;

  class Babinet : public OpticalElement {
      EMMedium                   m_inputCrystal;
      EMMedium                   m_outputCrystal;
      
      TranslatedFrame           *m_inputFrame     = nullptr;
      RotatedFrame              *m_innerFrame     = nullptr;
      TranslatedFrame           *m_outputFrame    = nullptr;

      RectangularWindowBoundary *m_inputBoundary  = nullptr;
      RectangularWindowBoundary *m_innerBoundary  = nullptr;
      RectangularWindowBoundary *m_outputBoundary = nullptr;

      Real m_width     = 5e-2; // [m]
      Real m_height    = 5e-2; // [m] 
      Real m_thickness = 1e-2; // [m]
      Real m_angle     = 5;    // [deg]

      Real m_h0, m_h1;         // Wedge heights
      
      GLWedge          m_wedge;
      
      Real             m_slopeLength = 0;

      void recalcModel();
      void recalcMedium();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      Babinet(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual ~Babinet() override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_OPTICAL_ELEMENT(Babinet);
}

#endif // _ELEMENTS_BABINET_H
