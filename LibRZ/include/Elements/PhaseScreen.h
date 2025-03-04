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

#ifndef _PHASE_SCREEN_H
#define _PHASE_SCREEN_H

#include <OpticalElement.h>
#include <MediumBoundaries/PhaseScreen.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class PhaseScreen : public OpticalElement {
      PhaseScreenBoundary *m_boundary;
      GLDisc    m_skyDiscFront;
      GLDisc    m_skyDiscBack;

      TranslatedFrame *m_tSurface = nullptr;
      Real m_muIn    = 1;
      Real m_muOut   = 1.5;
      Real m_radius  = 2.5e-2;
      
      GLuint m_textureId;
      std::vector<uint8_t> m_textureData;
      bool m_texDirty = true;

      void uploadTexture();
      void recalcModel();
      void recalcTexture();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      PhaseScreen(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~PhaseScreen() override;

      virtual void enterOpenGL() override;
      virtual void nativeMaterialOpenGL(std::string const &role) override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_OPTICAL_ELEMENT(PhaseScreen);
}


#endif // _PHASE_SCREEN_H
