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

#ifndef _IDEAL_LENS_H
#define _IDEAL_LENS_H

#include <OpticalElement.h>
#include <MediumBoundaries/IdealLens.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;
  class IdealLensBoundary;

  class IdealLens : public OpticalElement {
      GLCappedCylinder    m_cylinder;
      IdealLensBoundary  *m_boundary         = nullptr;
      TranslatedFrame    *m_inputFrame       = nullptr;
      
      TranslatedFrame    *m_frontFocalPlane  = nullptr;
      TranslatedFrame    *m_backFocalPlane   = nullptr;

      TranslatedFrame    *m_objectPlane      = nullptr;
      TranslatedFrame    *m_imagePlane       = nullptr;

      Real m_fLen      = 1.;
      Real m_radius    = 1e-2;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      IdealLens(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~IdealLens() override;

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_OPTICAL_ELEMENT(IdealLens);
}

#endif // _IDEAL_LENS_H
