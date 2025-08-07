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

#ifndef _BIREFRINGENT_PRISM_H
#define _BIREFRINGENT_PRISM_H

#include <OpticalElement.h>
#include <MediumBoundaries/RectangularWindow.h>
#include <GLHelpers.h>
#include <EMInterface.h>

namespace RZ {
  class TranslatedFrame;
  class RectangularWindowBoundary;

  class BirefringentPrism : public OpticalElement {
      EMMedium                   m_glass;
      RectangularWindowBoundary *m_inputBoundary  = nullptr;
      RectangularWindowBoundary *m_outputBoundary = nullptr;
      TranslatedFrame           *m_inputFrame     = nullptr;
      TranslatedFrame           *m_outputFrame    = nullptr;

      Real m_length  = 5e-2;
      Real m_width   = 3e-2;
      Real m_height  = 1e-2;
      Real m_axis[3] = {1, 0, 0};

      void recalcModel();
      void recalcMedium();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      BirefringentPrism(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~BirefringentPrism() override;

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_OPTICAL_ELEMENT(BirefringentPrism);
}

#endif // _BIREFRINGENT_PRISM_H
