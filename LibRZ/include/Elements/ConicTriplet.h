//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//  Copyright (c) 2025 Pablo Álvarez Martín
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

#ifndef _CONIC_TRIPLET_H
#define _CONIC_TRIPLET_H

#include <OpticalElement.h>
#include <MediumBoundaries/ConicLens.h>
#include <GLHelpers.h>
#include <EMInterface.h>

namespace RZ {
  class TranslatedFrame;

  class ConicTriplet : public OpticalElement {
      GLCappedCylinder        m_cylinder;
      GLConicCap              m_frontCap, m_middleCap1, m_middleCap2, m_backCap;
      EMMedium                m_glass1;
      EMMedium                m_glass2;
      EMMedium                m_glass3;
      ConicLensBoundary      *m_inputBoundary    = nullptr;
      ConicLensBoundary      *m_middleBoundary1  = nullptr;
      ConicLensBoundary      *m_middleBoundary2  = nullptr;
      ConicLensBoundary      *m_outputBoundary   = nullptr;

      TranslatedFrame        *m_inputFrame       = nullptr;
      TranslatedFrame        *m_middleFrame1     = nullptr;
      TranslatedFrame        *m_middleFrame2     = nullptr;
      TranslatedFrame        *m_outputFrame      = nullptr;

      TranslatedFrame        *m_objectPlane      = nullptr;
      TranslatedFrame        *m_middlePlane1     = nullptr;
      TranslatedFrame        *m_middlePlane2     = nullptr;
      TranslatedFrame        *m_imagePlane       = nullptr;

      // Per-surface properties
      Real m_K[4]           = {0, 0, 0, 0};
      Real m_rCurv[4]       = {1e-1, 1e-1, 1e-1, 1e-1};
      
      // Common properties
      Real m_radius         = 2.5e-2;
      Real m_x0             = 0;
      Real m_y0             = 0;
      
      Real m_thickness1      = 3e-2;
      Real m_edgeThickness1  = 1e-2;
      bool m_fromEdge1       = false;
      Real m_thickness2      = 3e-2;
      Real m_edgeThickness2  = 1e-2;
      bool m_fromEdge2       = false;
      Real m_thickness3      = 3e-2;
      Real m_edgeThickness3  = 1e-2;
      bool m_fromEdge3       = false;
      
      // Calculated properties
      Real m_displacement[4];
      
      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      ConicTriplet(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~ConicTriplet() override;

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_OPTICAL_ELEMENT(ConicTriplet);
}

#endif // _CONIC_TRIPLET_H
