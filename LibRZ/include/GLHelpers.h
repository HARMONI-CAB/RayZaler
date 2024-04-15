#ifndef _GLHELPERS_H
#define _GLHELPERS_H

#include <GL/glu.h>
#include <vector>
#include <Vector.h>
#include <ReferenceFrame.h>

namespace RZ {
  struct Vec3;

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

  // TODO: Remove GLUT altogether
  
  void GLCube(GLfloat);
  
  struct GLPrimitive {
    virtual void display() = 0;
  };

  class GLCone : public GLPrimitive {
      GLUquadric *m_quadric = nullptr;
      GLdouble m_base;
      GLdouble m_height;
      GLint    m_slices;
      GLint    m_stacks;

    public:
      GLCone();
      ~GLCone();

      void setBase(GLdouble);
      void setHeight(GLdouble);
      void setSlices(GLint);
      void setStacks(GLint);
      virtual void display() override;
  };

  class GLDisc : public GLPrimitive {
      GLUquadric *m_quadric = nullptr;
      bool m_dirty = true;
      std::vector<GLfloat> m_vertices;
      std::vector<GLfloat> m_normals;
      std::vector<GLfloat> m_texCoords;
      std::vector<GLint>   m_indices;

      GLint    m_slices = 32;
      GLdouble m_width  = 1;
      GLdouble m_height = 1;
      bool m_invertNormals = false;

      void recalculate();

    public:
      GLdouble
      width() const
      {
        return m_width;
      }

      GLdouble
      height() const
      {
        return m_height;
      }

      void setInverted(bool);
      void setRadius(GLdouble);
      void setWidth(GLdouble);
      void setHeight(GLdouble);
      void setSlices(GLint);

      virtual void display() override;

      GLDisc();
      ~GLDisc();
  };

  class GLRing : public GLPrimitive {
      GLUquadric *m_quadric = nullptr;
      bool m_dirty = true;
      std::vector<GLfloat> m_vertices;
      std::vector<GLfloat> m_normals;
      std::vector<GLfloat> m_texCoords;
      std::vector<GLint>   m_indices;

      GLint    m_slices      = 32;
      GLdouble m_innerRadius = .5;
      GLdouble m_outerRadius = 1.;

      void recalculate();

    public:
      GLdouble
      innerRadius() const
      {
        return m_innerRadius;
      }

      GLdouble
      outerRadius() const
      {
        return m_outerRadius;
      }

      void setInnerRadius(GLdouble);
      void setOuterRadius(GLdouble);
      void setSlices(GLint);

      virtual void display() override;

      GLRing();
      ~GLRing();
  };

  class GLCappedCylinder : public GLPrimitive {
      GLUquadric *m_quadric = nullptr;
      bool m_dirty = true;

      GLDisc   m_topCap, m_bottomCap;

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

  class GLTube : public GLPrimitive {
      GLUquadric *m_outerQuadric = nullptr;
      GLUquadric *m_innerQuadric = nullptr;
      bool     m_dirty = true;

      GLRing   m_topCap, m_bottomCap;

      bool     m_drawTop  = false;
      bool     m_drawBase = false;
      GLdouble m_height = 1.;
      GLdouble m_innerRadius = .125;
      GLdouble m_outerRadius = .25;
      GLint    m_slices = 32;

      void recalculateCaps();

    public:
      GLdouble
      height() const
      {
        return m_height;
      }

      GLdouble
      innerRadius() const
      {
        return m_innerRadius;
      }

      GLdouble
      outerRadius() const
      {
        return m_outerRadius;
      }

      void setHeight(GLdouble);
      void setInnerRadius(GLdouble);
      void setOuterRadius(GLdouble);
      void setSlices(GLint);
      void setVisibleCaps(bool, bool);

      virtual void display() override;

      GLTube();
      ~GLTube();
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

  class GLParabolicCap : public GLPrimitive {
      GLUquadric *m_quadric = nullptr;
      bool m_dirty = true;
      std::vector<GLfloat> m_vertices;
      std::vector<GLfloat> m_normals;
      std::vector<GLfloat> m_texCoords;
      std::vector<GLint>   m_indices;

      GLdouble m_flength    = 2;
      GLdouble m_radius  = .25;
      GLint    m_sectors = 32;
      GLint    m_stacks  = 8;
      bool     m_invertNormals = false;

      void recalculate();

    public:
      GLdouble
      fnum() const
      {
        return m_flength;
      }

      GLdouble
      radius() const
      {
        return m_radius;
      }
      
      void setFocalLength(GLdouble);
      void setRadius(GLdouble);
      void setSectors(GLint);
      void setStacks(GLint);
      void setInvertNormals(bool);

      virtual void display() override;

      GLParabolicCap();
      ~GLParabolicCap();
  };

  class GLPinHole : public GLPrimitive {
      GLUquadric *m_quadric = nullptr;
      bool m_dirty = true;
      std::vector<GLfloat> m_vertices;
      std::vector<GLfloat> m_normals;

      GLint    m_slices = 32;
      GLdouble m_radius = .25;
      GLdouble m_width  = 1;
      GLdouble m_height = 1;

      void recalculate();

    public:
      GLdouble
      radius() const
      {
        return m_radius;
      }

      GLdouble
      width() const
      {
        return m_width;
      }

      GLdouble
      height() const
      {
        return m_height;
      }

      void setRadius(GLdouble);
      void setWidth(GLdouble);
      void setHeight(GLdouble);
      void setSlices(GLint);

      virtual void display() override;

      GLPinHole();
      ~GLPinHole();
  };

  class GLRectangle : public GLPrimitive {
      bool m_dirty = true;
      std::vector<GLfloat> m_vertices;

      GLdouble m_width  = 1;
      GLdouble m_height = 1;

      void recalculate();

    public:
      GLdouble
      width() const
      {
        return m_width;
      }

      GLdouble
      height() const
      {
        return m_height;
      }

      void setWidth(GLdouble);
      void setHeight(GLdouble);

      virtual void display() override;

      GLRectangle();
      ~GLRectangle();
  };


  class GLReferenceFrame : public GLPrimitive {
      GLCappedCylinder     m_axisCylinder;
      GLCone               m_axisArrow;

      GLfloat              m_height = 1e-1;
      GLfloat              m_radius = 5e-3;
      GLfloat              m_arrowHeight = 1e-1 / 3;
      GLfloat              m_arrowBase   = 1e-2;

    public:
      void setHeight(GLfloat);
      void setRadius(GLfloat);
      void setArrowHeight(GLfloat);
      void setArrowBase(GLfloat);

      GLfloat
      height() const
      {
        return m_height;
      }

      GLfloat
      radius() const
      {
        return m_radius;
      }

      GLfloat
      arrowHeight() const
      {
        return m_arrowHeight;
      }

      GLfloat
      arrowBase() const
      {
        return m_arrowBase;
      }

      virtual void display() override;

      GLReferenceFrame();
      ~GLReferenceFrame();
  };
  
  class GLArrow : public GLPrimitive {
      Vec3            m_direction;
      GLfloat         m_thickness = 2;
      
    public:
      void setThickness(GLfloat thickness);
      void setDirection(Vec3 const &vec);
      
      virtual void display() override;

      GLArrow();
      ~GLArrow(); 
  };
};

#endif // _GLHELPERS_H
