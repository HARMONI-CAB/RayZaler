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

#ifndef _OBSTRUCTION_H
#define _OBSTRUCTION_H

#include <OpticalElement.h>
#include <MediumBoundaries/Obstruction.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;
 
  struct Vertex {
    float vertexCoords[3];
    float texCoords[2];
  };

  class Obstruction : public OpticalElement {
      ObstructionProcessor *m_processor;
      GLDisc                m_disc;
      TranslatedFrame      *m_stopSurface = nullptr;
      std::vector<Real>     m_obstructionMap;
      GLShader             *m_alphaTestShader = nullptr;
      GLuint                m_verCoordAttrib = 0;
      GLuint                m_texCoordAttrib = 0;
      GLuint                m_textureUniformId = 0;
      GLuint                m_colorUniformId = 0;

      std::string           m_path;
      bool                  m_openGlInitilized = false;
      unsigned int          m_cols   = 0;
      unsigned int          m_rows   = 0;
      unsigned int          m_stride = 0;

      Real                  m_halfMapWidth  = 0;
      Real                  m_halfMapHeight = 0;
      Real m_radius = 1e-2;

      Vertex                m_obsVertices[4];
      uint16_t              m_obsIndices[12] = {0, 1, 2, 2, 3, 0, 0, 3, 2, 2, 1, 0};
      std::vector<uint8_t>  m_textureData;
      GLuint                m_textureId;
      GLuint                m_vaoId;
      GLuint                m_vboId;
      GLuint                m_iboId;
      bool                  m_texDirty = false;

      void recalcModel();
      void setFromPNG(std::string const &path);
      void rebuildTexture();
      void initOpenGLObjects();
      void uploadAll();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      Obstruction(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~Obstruction() override;

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;
      virtual void enterOpenGL() override;
  };

  RZ_DECLARE_OPTICAL_ELEMENT(Obstruction);
}

#endif // _OBSTRUCTION_H
