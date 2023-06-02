#ifndef _RZ_GL_MODEL_H
#define _RZ_GL_MODEL_H

#include <GLModel.h>
#include <list>

namespace RZ {
  class Element;
  class OMModel;

  class RZGLModel : public GLModel {
      std::list<Element *> m_elements;
      GLfloat m_refMatrix[16];
      
      void pushElementMatrix(Element *);
      void popElementMatrix();

    public:
      virtual void display() override;
      void pushElement(Element *);
      void pushOptoMechanicalModel(OMModel *);
  };
}

#endif // _RZ_GL_MODEL_H
