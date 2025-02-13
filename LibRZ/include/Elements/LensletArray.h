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

#ifndef _LENSLET_ARRAY_H
#define _LENSLET_ARRAY_H

#include <OpticalElement.h>
#include <MediumBoundaries/LensletArray.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;  

  class LensletArray : public OpticalElement {
      GLCappedCylinder        m_cylinder;
      GLSphericalCap          m_cap;
      LensletArrayBoundary  *m_inputBoundary   = nullptr;
      LensletArrayBoundary  *m_outputBoundary  = nullptr;
      TranslatedFrame        *m_inputFrame       = nullptr;
      TranslatedFrame        *m_outputFrame      = nullptr;

      TranslatedFrame        *m_inputFocalPlane  = nullptr;
      TranslatedFrame        *m_outputFocalPlane = nullptr;

      TranslatedFrame        *m_objectPlane      = nullptr;
      TranslatedFrame        *m_imagePlane       = nullptr;

      Real m_thickness = 1e-2;
      Real m_width     = 1e-1;
      Real m_height    = 1e-1;
      Real m_K         = 0;
      unsigned m_rows  = 10;
      unsigned m_cols  = 10;
      Real m_rCurv     = 1;
      Real m_mu        = 1.5;
      Real m_depth     = 0;
      Real m_f         = 0;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      LensletArray(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~LensletArray() override;

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_OPTICAL_ELEMENT(LensletArray);
}

#endif // _LENSLET_ARRAY_H
