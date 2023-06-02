#ifndef _GL_MODEL_H
#define _GL_MODEL_H

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

namespace RZ {
  class GLModelEventListener {
    public:
      virtual void tick() = 0;
  };

  class GLModel {
      GLModelEventListener *m_listener = nullptr;

    protected:
      void tick();
    
    public:
      void setEventListener(GLModelEventListener *listener);
      virtual void configureLighting();
      virtual void display() = 0;
  };
}

#endif // _GL_MODEL_H
