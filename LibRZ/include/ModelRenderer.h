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
