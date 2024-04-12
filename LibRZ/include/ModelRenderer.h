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

  class ModelRenderer : public GLRenderEngine {
      OSMesaContext        m_ctx;
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
      ModelRenderer(unsigned int width, unsigned int height);
      ~ModelRenderer();

      void zoom(GLfloat delta);
      void incAzEl(GLfloat deltaAz, GLfloat deltaEl);
      void roll(GLfloat delta);
      void move(GLfloat deltaX, GLfloat deltaY);

      void setZoom(GLfloat delta);
      void setCenter(GLfloat, GLfloat);
      void setRotation(GLfloat, GLfloat, GLfloat, GLfloat);
      
      void pushOptoMechanicalModel(OMModel *);
      void render();
      bool savePNG(const char *path);
  };
}

#endif // _RZ_MODEL_RENDERER_H
