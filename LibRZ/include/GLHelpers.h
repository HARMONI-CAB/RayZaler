#ifndef _GLHELPERS_H
#define _GLHELPERS_H

#include <GL/glu.h>
#include <vector>

namespace RZ {
  struct GLVectorStorage {
    GLfloat m_params[4];

    inline GLfloat *
    get(GLfloat x, GLfloat y, GLfloat z, GLfloat t = 1.)
    {
      m_params[0] = x;
      m_params[1] = y;
      m_params[2] = z;
      m_params[3] = t;
      
      return m_params;
    }
  };


  struct GLPrimitive {
    virtual void display() = 0;
  };

  class GLCappedCylinder : public GLPrimitive {
      GLUquadric *m_quadric = nullptr;
      bool m_dirty = true;
      std::vector<GLfloat> m_topCapVertices;
      std::vector<GLfloat> m_baseCapVertices;

      bool     m_drawTop  = false;
      bool     m_drawBase = false;
      GLdouble m_height = 1.;
      GLdouble m_radius = .25;
      GLint    m_slices = 32;

      void recalculateCaps();

    public:
      void setHeight(GLdouble);
      void setRadius(GLdouble);
      void setSlices(GLint);
      void setVisibleCaps(bool, bool);

      virtual void display() override;

      GLCappedCylinder();
      ~GLCappedCylinder();
  };
};

#endif // _GLHELPERS_H
