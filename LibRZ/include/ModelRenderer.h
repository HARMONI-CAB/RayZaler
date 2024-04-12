#ifndef _RZ_MODEL_RENDERER_H
#define _RZ_MODEL_RENDERER_H

#include <list>
#include <vector>
#include <cstdint>
#include <GL/gl.h>
#include <GL/osmesa.h>
#include "GLRenderEngine.h"
#include "IncrementalRotation.h"

namespace RZ {
  class OMModel;
  class Element;
  class GLModel;

  class ModelRenderer : public GLRenderEngine {
      OSMesaContext        m_ctx;
      GLModel           *m_ownModel;
      unsigned int m_width = 680;
      unsigned int m_height = 480;
      bool m_fixedLight = false;
      GLfloat m_zoom = 1;
      GLfloat m_currentCenter[2] = {0, 0};
      IncrementalRotation m_incRot;
      std::vector<uint32_t> m_pixels;
      void showScreen();
      void adjustViewPort();

    public:
      ModelRenderer(unsigned int width, unsigned int height, GLModel *own = nullptr);
      ~ModelRenderer();

      inline unsigned
      width() const {
        return m_width;
      }

      inline unsigned
      height() const {
        return m_height;
      }
      
      void zoom(Real delta);
      void incAzEl(Real deltaAz, Real deltaEl);
      void roll(Real delta);
      void move(Real deltaX, Real deltaY);

      void setZoom(Real delta);
      void setCenter(Real, Real);
      void setRotation(Real, Real, Real, Real);
      
      void render();
      bool savePNG(const char *path);
      const uint32_t *pixels() const;

      static ModelRenderer *fromOMModel(OMModel *, unsigned, unsigned);
  };
}

#endif // _RZ_MODEL_RENDERER_H
