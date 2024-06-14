#ifndef _GL_MODEL_H
#define _GL_MODEL_H

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "IncrementalRotation.h"
#include "Element.h"

namespace RZ {
  class GLModelEventListener {
    public:
      virtual void tick() = 0;
  };

  class GLModel {
      GLModelEventListener *m_listener = nullptr;
      GLfloat                m_refMatrix[16];
      GLfloat                m_apertureColor[4] = {0, 0, 1, 1};
      
    protected:
      void tick();
      void drawElementApertures(const Element *);
      
    public:
      void setOrientationAndCenter(RZ::Matrix3 const &R, RZ::Vec3 const &O);
      void pushReferenceFrameMatrix(const RZ::ReferenceFrame *frame);
      void pushElementMatrix(Element *);
      void popElementMatrix();

      GLfloat *refMatrix();
      void updateRefMatrix();
      
      void setApertureColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
      void setApertureColor(const GLfloat *rgb);

      void setEventListener(GLModelEventListener *listener);
      virtual void configureLighting();
      virtual void display() = 0;
  };
}

#endif // _GL_MODEL_H
