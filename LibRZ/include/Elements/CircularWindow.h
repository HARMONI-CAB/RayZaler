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

#ifndef _CIRCULAR_WINDOW_H
#define _CIRCULAR_WINDOW_H

#include <OpticalElement.h>
#include <MediumBoundaries/CircularWindow.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;
  class CircularWindowBoundary;

  class CircularWindow : public OpticalElement {
      GLCappedCylinder         m_cylinder;
      CircularWindowBoundary *m_inputBoundary   = nullptr;
      CircularWindowBoundary *m_outputBoundary  = nullptr;
      TranslatedFrame         *m_inputFrame       = nullptr;
      TranslatedFrame         *m_outputFrame      = nullptr;

      Real m_thickness = 1e-2;
      Real m_radius    = 1e-2;
      Real m_mu        = 1.5;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      CircularWindow(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~CircularWindow() override;

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_OPTICAL_ELEMENT(CircularWindow);
}

#endif // _CIRCULAR_WINDOW_H
