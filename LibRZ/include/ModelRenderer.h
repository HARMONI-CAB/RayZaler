#ifndef _RZ_MODEL_RENDERER_H
#define _RZ_MODEL_RENDERER_H

#include <list>
#include <vector>
#include <cstdint>
#include <GL/gl.h>

namespace RZ {
  class OMModel;
  class Element;

  class ModelRenderer {
      std::list<Element *> m_renderList;
      std::list<Element *> m_beams;

      unsigned int m_width = 680;
      unsigned int m_height = 480;
      GLfloat m_refMatrix[16];

      std::vector<uint32_t> m_pixels;
      GLuint m_fbo;
      GLuint m_renderBuf;

      void pushElementMatrix(Element *);
      void popElementMatrix();
      void renderElements(std::list<Element *> const &);

    public:
      ModelRenderer(
        unsigned int width,
        unsigned int height,
        OMModel *model = nullptr);
      ~ModelRenderer();

      void pushElement(Element *);
      void pushModel(OMModel *);
      void render();
      bool savePNG(const char *path);
  };
}

#endif // _RZ_MODEL_RENDERER_H
