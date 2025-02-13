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

#ifndef _CONIC_LENS_H
#define _CONIC_LENS_H

#include <OpticalElement.h>
#include <MediumBoundaries/ConicLens.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class ConicLens : public OpticalElement {
      GLCappedCylinder        m_cylinder;
      GLConicCap              m_frontCap, m_backCap;
      ConicLensBoundary      *m_inputBoundary    = nullptr;
      ConicLensBoundary      *m_outputBoundary   = nullptr;
      TranslatedFrame        *m_inputFrame       = nullptr;
      TranslatedFrame        *m_outputFrame      = nullptr;

      TranslatedFrame        *m_frontFocalPlane  = nullptr;
      TranslatedFrame        *m_backFocalPlane   = nullptr;

      TranslatedFrame        *m_objectPlane      = nullptr;
      TranslatedFrame        *m_imagePlane       = nullptr;

      // Per-surface properties
      Real m_K[2]           = {0, 0};
      Real m_focalLength[2] = {5e-2, 5e-2};
      Real m_rCurv[2]       = {1e-1, 1e-1};
      bool m_fromFlen[2]    = {false, false};
      
      // Common properties
      Real m_radius         = 2.5e-2;
      Real m_x0             = 0;
      Real m_y0             = 0;
      Real m_mu             = 1.5;
      Real m_thickness      = 1e-2;
      
      // Calculated properties
      Real m_displacement[2];
      
      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      ConicLens(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~ConicLens() override;

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_OPTICAL_ELEMENT(ConicLens);
}

#endif // _CONIC_LENS_H
