#ifndef _RZ_GL_MODEL_H
#define _RZ_GL_MODEL_H

#include <GLModel.h>
#include <list>

namespace RZ {
  class Element;
  class OMModel;

  class RZGLModel : public GLModel {
      OMModel *m_model = nullptr; // Borrowed
      bool     m_showApertures = false;
      bool     m_showElements = true;

      void displayModel(OMModel *);

    public:
      virtual void display() override;
      void pushOptoMechanicalModel(OMModel *);
      void setShowApertures(bool);
      void setShowElements(bool);
  };
}

#endif // _RZ_GL_MODEL_H
