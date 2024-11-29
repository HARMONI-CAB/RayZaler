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
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class ConicLens : public OpticalElement {
      GLCappedCylinder        m_cylinder;
      GLConicCap              m_topCap, m_bottomCap;
      ConicLensProcessor     *m_inputProcessor   = nullptr;
      ConicLensProcessor     *m_outputProcessor  = nullptr;
      TranslatedFrame        *m_inputFrame       = nullptr;
      TranslatedFrame        *m_outputFrame      = nullptr;

      TranslatedFrame        *m_frontFocalPlane  = nullptr;
      TranslatedFrame        *m_backFocalPlane = nullptr;

      TranslatedFrame        *m_objectPlane      = nullptr;
      TranslatedFrame        *m_imagePlane       = nullptr;

      Real m_radius  = .5;
      Real m_K       = 0;
      Real m_rCurv   = 1;
      Real m_rHole   = 0;
      Real m_rHole2  = 0;
      Real m_x0      = 0;
      Real m_y0      = 0;
      
      Real m_mu      = 1.5;
      Real m_displacement;
      Real m_thickness = 5e-3;
      
      Real m_focalLength;
      bool m_fromFlen;
      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      ConicLens(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~ConicLens();

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  class ConicLensFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _CONIC_LENS_H
