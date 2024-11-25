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

#ifndef _CONVEX_LENS_H
#define _CONVEX_LENS_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class ConvexLens : public OpticalElement {
      GLCappedCylinder        m_cylinder;
      GLSphericalCap          m_topCap, m_bottomCap;
      SphericalLensProcessor *m_inputProcessor   = nullptr;
      SphericalLensProcessor *m_outputProcessor  = nullptr;
      TranslatedFrame        *m_inputFrame       = nullptr;
      TranslatedFrame        *m_outputFrame      = nullptr;

      TranslatedFrame        *m_inputFocalPlane  = nullptr;
      TranslatedFrame        *m_outputFocalPlane = nullptr;

      TranslatedFrame        *m_objectPlane      = nullptr;
      TranslatedFrame        *m_imagePlane       = nullptr;

      Real m_thickness = 1e-2;
      Real m_radius    = 1e-2;
      Real m_rCurv     = 1;
      Real m_mu        = 1.5;
      Real m_depth     = 0;
      Real m_f         = 0;
      bool m_fromFlen  = false;
      
      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      ConvexLens(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~ConvexLens();

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  class ConvexLensFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _CONVEX_LENS_H
