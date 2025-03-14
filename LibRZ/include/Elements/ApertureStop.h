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

#ifndef _APERTURE_STOP_H
#define _APERTURE_STOP_H

#include <OpticalElement.h>
#include <MediumBoundaries/ApertureStop.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class ApertureStop : public OpticalElement {
      ApertureStopBoundary *m_boundary;
      GLPinHole             m_pinHole;
      TranslatedFrame      *m_stopSurface = nullptr;
      
      Real m_radius   = 1e-2;
      Real m_width    = 3e-2;
      Real m_height   = 3e-2;
      bool m_infinite = true;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      ApertureStop(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~ApertureStop() override;

      virtual void nativeMaterialOpenGL(std::string const &role) override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_OPTICAL_ELEMENT(ApertureStop);
}

#endif // _APERTURE_STOP_H
