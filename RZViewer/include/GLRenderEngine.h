#ifndef _GL_RENDER_ENGINE_H
#define _GL_RENDER_ENGINE_H

namespace RZ {
  class GLModel;

  class GLRenderEngine {
      GLModel *m_model = nullptr; // Borrowed

    public:
      void setModel(GLModel *);
      GLModel *model() const;
  };
}

#endif // _GL_RENDER_ENGINE_H

