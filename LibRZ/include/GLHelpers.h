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
      GLdouble
      height() const
      {
        return m_height;
      }

      GLdouble
      radius() const
      {
        return m_radius;
      }

      void setHeight(GLdouble);
      void setRadius(GLdouble);
      void setSlices(GLint);
      void setVisibleCaps(bool, bool);

      virtual void display() override;

      GLCappedCylinder();
      ~GLCappedCylinder();
  };

  class GLSphericalCap : public GLPrimitive {
      GLUquadric *m_quadric = nullptr;
      bool m_dirty = true;
      std::vector<GLfloat> m_vertices;
      std::vector<GLfloat> m_normals;
      std::vector<GLfloat> m_texCoords;
      std::vector<GLint>   m_indices;

      GLdouble m_height = .125;
      GLdouble m_radius = .25;
      GLint    m_sectors = 32;
      GLint    m_stacks = 8;
      bool     m_invertNormals = false;
      
      void recalculate();

    public:
      GLdouble
      height() const
      {
        return m_height;
      }

      GLdouble
      radius() const
      {
        return m_radius;
      }
      
      void setHeight(GLdouble);
      void setRadius(GLdouble);
      void setSectors(GLint);
      void setStacks(GLint);
      void setInvertNormals(bool);

      virtual void display() override;

      GLSphericalCap();
      ~GLSphericalCap();
  };

};

#endif // _GLHELPERS_H
