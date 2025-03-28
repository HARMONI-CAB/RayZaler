//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

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
  
  class GLShader {
    int m_id = -1;
    bool m_initialized = false;
    bool checkBuildErrors(unsigned int shader, std::string const &type);

public:
    GLShader(const char * , const char *);
    ~GLShader();

    inline int
    program() const
    {
      return m_id;
    }

    void use();
    void leave();

    void set(std::string const &, bool) const;
    void set(std::string const &, int) const;
    void set(std::string const &, unsigned int) const;
    void set(std::string const &, Vec3 const &) const;
    void set(std::string const &, Real) const;
};

  void GLCube(GLfloat, bool wireFrame = false);

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

  class GLAbstractCap : public GLPrimitive {
    public:
      virtual void requestRecalc();
      virtual const std::vector<GLfloat> *edge() const = 0; 
  };

  class GLDisc : public GLAbstractCap {
      GLUquadric *m_quadric = nullptr;
      bool m_dirty = true;
      std::vector<GLfloat> m_vertices;
      std::vector<GLfloat> m_normals;
      std::vector<GLfloat> m_texCoords;
      std::vector<GLfloat> m_edge;
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

      virtual void requestRecalc() override;
      void setInverted(bool);
      void setRadius(GLdouble);
      void setWidth(GLdouble);
      void setHeight(GLdouble);
      void setSlices(GLint);

      const std::vector<GLfloat> *edge() const override;
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

      GLDisc   m_topDiscCap, m_bottomDiscCap;
      GLAbstractCap *m_topCap = nullptr;
      GLAbstractCap *m_bottomCap = nullptr;

      std::vector<GLfloat> m_strip;
      std::vector<GLfloat> m_normals;
      
      bool     m_drawTop  = false;
      bool     m_drawBase = false;
      bool     m_invertNormals = false;
      GLdouble m_height   = 1.;
      GLdouble m_radius   = .25;
      GLint    m_slices   = 64;

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

      void setCaps(GLAbstractCap *top, GLAbstractCap *bottom);
      void setHeight(GLdouble);
      void setRadius(GLdouble);
      void setSlices(GLint);
      void setVisibleCaps(bool, bool);
      void setInvertNormals(bool);
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

  class GLSphericalCap : public GLAbstractCap {
      GLUquadric *m_quadric = nullptr;
      bool m_dirty = true;
      std::vector<GLfloat> m_vertices;
      std::vector<GLfloat> m_normals;
      std::vector<GLfloat> m_texCoords;
      std::vector<GLint>   m_indices;
      std::vector<GLfloat> m_edge;

      GLdouble m_rCurv   = 1;
      GLdouble m_radius  = .25;
      GLdouble m_x0      = 0;
      GLdouble m_y0      = 0;
      GLint    m_sectors = 32;
      GLint    m_stacks  = 8;
      bool     m_invertNormals = false;

      void recalculate();

    public:
      GLdouble
      curvatureRadius() const
      {
        return m_rCurv;
      }

      GLdouble
      radius() const
      {
        return m_radius;
      }
      
      void setCenterOffset(GLdouble, GLdouble);
      void setCurvatureRadius(GLdouble);
      void setRadius(GLdouble);
      void setSectors(GLint);
      void setStacks(GLint);
      void setInvertNormals(bool);

      virtual void display() override;
      virtual const std::vector<GLfloat> *edge() const override;
      virtual void requestRecalc() override;

      GLSphericalCap();
      ~GLSphericalCap();
  };

  class GLParabolicCap : public GLAbstractCap {
      GLUquadric *m_quadric = nullptr;
      bool m_dirty = true;
      std::vector<GLfloat> m_vertices;
      std::vector<GLfloat> m_normals;
      std::vector<GLfloat> m_texCoords;
      std::vector<GLint>   m_indices;
      std::vector<GLfloat> m_edge;

      GLdouble m_flength = 2;
      GLdouble m_radius  = .25;
      GLdouble m_x0      = 0;
      GLdouble m_y0      = 0;
      GLint    m_sectors = 64;
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
      
      void setCenterOffset(GLdouble, GLdouble);
      void setFocalLength(GLdouble);
      void setRadius(GLdouble);
      void setSectors(GLint);
      void setStacks(GLint);
      void setInvertNormals(bool);

      virtual const std::vector<GLfloat> *edge() const override;
      virtual void requestRecalc() override;
      virtual void display() override;
      
      GLParabolicCap();
      ~GLParabolicCap();
  };

  class GLConicCap : public GLAbstractCap {
      GLUquadric *m_quadric = nullptr;
      bool m_dirty = true;
      std::vector<GLfloat> m_vertices;
      std::vector<GLfloat> m_normals;
      std::vector<GLfloat> m_texCoords;
      std::vector<GLint>   m_indices;
      std::vector<GLfloat> m_edge;

      GLdouble m_rCurv   = 1;
      GLdouble m_K       = 0;
      bool     m_convex  = false;
      GLdouble m_radius  = .25;
      GLdouble m_x0      = 0;
      GLdouble m_y0      = 0;
      GLdouble m_rHole   = 0;
      GLint    m_sectors = 64;
      GLint    m_stacks  = 8;
      bool     m_invertNormals = false;

      void recalculate();

    public:
      GLdouble
      fnum() const
      {
        return m_rCurv / 2;
      }

      GLdouble
      radius() const
      {
        return m_radius;
      }
      
      void setCenterOffset(GLdouble, GLdouble);
      void setConicConstant(GLdouble);
      void setCurvatureRadius(GLdouble);
      void setConvex(bool);

      void setRadius(GLdouble);
      void setHoleRadius(GLdouble);
      void setSectors(GLint);
      void setStacks(GLint);
      void setInvertNormals(bool);

      virtual const std::vector<GLfloat> *edge() const override;
      virtual void requestRecalc() override;
      virtual void display() override;
      
      GLConicCap();
      ~GLConicCap();
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
      Vec3            m_origin;
      GLfloat         m_thickness = 2;
      
    public:
      void setThickness(GLfloat thickness);
      void setOrigin(Vec3 const &vec);
      void setDirection(Vec3 const &vec);
      
      virtual void display() override;

      GLArrow();
      ~GLArrow(); 
  };

  class GLText : public GLPrimitive {
    unsigned int               m_texId;
    std::vector<uint32_t>      m_texture;
    std::string                m_text;
    std::string                m_face;
    GLfloat                    m_vertices[6][2];
    unsigned int               m_texWidth, m_texHeight;
    unsigned int               m_fontSize = 48;

    GLfloat                    m_scale = 1e-3;
    GLfloat                    m_color[4] = {1, 1, 1, 1};

    unsigned int m_size = 48;

    bool m_haveTexture = false;
    bool m_texLoaded = false;
    bool m_needsReload = false;

    void composeTexture();
    void reloadTexture();

  public:
    ~GLText();

    void setSize(unsigned);
    void setScale(GLfloat);
    void setColor(GLfloat, GLfloat, GLfloat, GLfloat = 1);
    void setText(std::string const &);
    void setFace(std::string const &);
    
    virtual void display() override;
  };

  class GLGrid : public GLPrimitive {
      std::vector<GLfloat> m_vertices;
      std::vector<GLfloat> m_hlVertices;
      GLfloat              m_gridColor[4] = {1, 1, 1, 1};
      GLfloat              m_hlColor[4] = {1, 0, 0, 1};

      unsigned             m_stepsX = 10;
      unsigned             m_stepsY = 10;
      Real                 m_step = 0.1;
      GLfloat              m_thickness = 1;
      GLfloat              m_x = 0;
      GLfloat              m_y = 0;

      void recalculateHighlight();
      void recalculate();

    public:
      inline GLfloat
      width() const
      {
        return m_stepsX * m_step;
      }
      
      inline GLfloat
      height() const
      {
        return m_stepsY * m_step;
      }

      inline GLfloat
      step() const
      {
        return m_step;
      }
      
      virtual void display() override;

      void setGridColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1);
      void setHighlightColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1);
      
      void setStepsX(unsigned);
      void setStepsY(unsigned);
      void setStep(Real);
      void setThickness(GLfloat);

      void highlight(GLfloat x, GLfloat y);

      GLGrid();
      ~GLGrid();
  };
};

#endif // _GLHELPERS_H
