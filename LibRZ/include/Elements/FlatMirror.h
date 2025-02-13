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

#ifndef _FLAT_MIRROR_H
#define _FLAT_MIRROR_H

#include <OpticalElement.h>
#include <MediumBoundaries/FlatMirror.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;
  class RotatedFrame;

  class FlatMirror : public OpticalElement {
      GLCappedCylinder m_cylinder;
      FlatMirrorBoundary *m_boundary;
      TranslatedFrame *m_reflectiveSurfaceFrame = nullptr;
      RotatedFrame    *m_flipFrame = nullptr;
      TranslatedFrame *m_baseFrame = nullptr;
      bool m_vertexRelative = false;
      Real m_thickness = 1e-2;
      Real m_radius = 1e-2;
      Real m_width  = 2e-2;
      Real m_height = 2e-2;
      Real m_a      = 1;
      Real m_b      = 1;
      
      // Calculated members
      Real m_ecc    = 0;

      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      FlatMirror(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~FlatMirror() override;

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_OPTICAL_ELEMENT(FlatMirror);
}

#endif // _FLAT_MIRROR_H
