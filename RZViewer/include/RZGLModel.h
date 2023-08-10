#ifndef _RZ_GL_MODEL_H
#define _RZ_GL_MODEL_H

#include <GLModel.h>
#include <list>

namespace RZ {
  class Element;
  class OMModel;

  class RZGLModel : public GLModel {
      OMModel *m_model = nullptr; // Borrowed
      GLfloat m_refMatrix[16];
      
      void pushElementMatrix(Element *);
      void popElementMatrix();

      void displayModel(OMModel *);

    public:
      virtual void display() override;
      void pushOptoMechanicalModel(OMModel *);
  };
}

#endif // _RZ_GL_MODEL_H
