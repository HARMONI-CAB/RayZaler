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

#ifndef _CONIC_MIRROR_H
#define _CONIC_MIRROR_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class ConicMirror : public OpticalElement {
      GLCappedCylinder m_cylinder;
      GLCappedCylinder m_hole;
      GLConicCap   m_cap, m_rearCap;
      ConicMirrorProcessor *m_processor;
      TranslatedFrame *m_reflectiveSurfaceFrame = nullptr;
      TranslatedFrame *m_reflectiveSurfacePort = nullptr;
      Real m_thickness = 1e-2;
      Real m_radius = 1e-2;
      Real m_K = 0;
      Real m_rCurv = 2e-2;
      bool m_convex = false;
      Real m_displacement = 0;
      Real m_x0 = 0;
      Real m_y0 = 0;
      Real m_rHole = .5;
      Real m_rHoleHeight = 0;
      
      void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      ConicMirror(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~ConicMirror() override;

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
  };

  class ConicMirrorFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _CONIC_MIRROR_H
