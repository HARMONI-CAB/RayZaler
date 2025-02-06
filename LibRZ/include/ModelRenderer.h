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

#ifndef _RZ_MODEL_RENDERER_H
#define _RZ_MODEL_RENDERER_H

#include <list>
#include <vector>
#include <cstdint>
#include <GL/gl.h>
#include <GL/osmesa.h>
#include "GLRenderEngine.h"

namespace RZ {
  class OMModel;
  class Element;
  class GLModel;
  class RZGLModel;

  class ModelRenderer : public GLRenderEngine {
      OSMesaContext         m_ctx;
      RZGLModel            *m_ownModel;
      unsigned int          m_width = 680;
      unsigned int          m_height = 480;
      bool                  m_fixedLight = false;

      std::vector<uint32_t> m_pixels;
      void showScreen();
      void adjustViewPort();

      ModelRenderer(unsigned int width, unsigned int height, RZGLModel *own = nullptr);

    public:
      ~ModelRenderer();

      inline unsigned
      width() const {
        return m_width;
      }

      inline unsigned
      height() const {
        return m_height;
      }
      
      void render();
      void zoomToContents();
      void zoomToElement(Element const *);
      void setHighlightedBoundingBox(Element *);
      void setShowElements(bool);
      void setShowApertures(bool);
      bool savePNG(const char *path);
      const uint32_t *pixels() const;

      static ModelRenderer *fromOMModel(
        OMModel *,
        unsigned,
        unsigned,
        bool showElements = true,
        bool showApertures = false);
  };
}

#endif // _RZ_MODEL_RENDERER_H
