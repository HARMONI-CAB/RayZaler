#ifndef _GL_RENDER_ENGINE_H
#define _GL_RENDER_ENGINE_H

#include "IncrementalRotation.h"
#include <GLHelpers.h>
#include <list>

namespace RZ {
  class GLModel;

  struct GLCurrentView {
    Real                  zoomLevel = 1;
    Real                  center[2] = {0, 0};
    Real                  width, height;
    IncrementalRotation   rotation;

    void zoom(Real delta);
    void incAzEl(Real deltaAz, Real deltaEl);
    void roll(Real delta);
    void move(Real deltaX, Real deltaY);

    void setScreenGeom(Real width, Real height);
    void setZoom(Real delta);
    void setCenter(Real, Real);
    void setRotation(Real, Real, Real, Real);
    void setRotation(RZ::Vec3 const &, Real);
    void setRotation(RZ::Matrix3 const &);
    void rotateRelative(RZ::Vec3 const &, Real);
    void configureViewPort(unsigned int width, unsigned int height) const;
    void configureOrientation(bool translate = true) const;

    void screenToWorld(Real &wX, Real &wY, Real sX, Real sY);
    void worldToScreen(Real &sX, Real &sY, Real wX, Real wY);
  };

  class GLHelperGrid {
    GLGrid           m_xyCoarseGrid;
    GLGrid           m_xyMediumGrid;
    GLGrid           m_xyFineGrid;
    GLText           m_glGridText;
    unsigned         m_divs = 0;
    GLfloat          m_step = 0;
    GLfloat          m_color[3] = {1, 1, 1};

    public:
      GLHelperGrid();

      void display();

      void setColor(GLfloat, GLfloat, GLfloat);
      void setColor(const GLfloat *);
      void setGridText(std::string const &);
      void setGridStep(Real step);
      void setGridDivs(unsigned int num);
      void highlight(Real, Real);
  };

  struct GLFrameGrid {
    ReferenceFrame *frame = nullptr; // Borrowed
    GLHelperGrid grid;
  };

  class GLRenderEngine {
      GLModel               *m_model = nullptr; // Borrowed
      GLCurrentView          m_view;
      GLReferenceFrame       m_glAxes;
      std::list<GLFrameGrid> m_grids;
      GLfloat                m_refMatrix[16];

    protected:
      void pushReferenceFrameMatrix(const RZ::ReferenceFrame *frame);
      void setOrientationAndCenter(RZ::Matrix3 const &R, RZ::Vec3 const &O);
      void drawCornerAxes();
      void drawReferenceFrames();

    public:
      GLCurrentView *view();
      GLfloat       *refMatrix();

      void addGrid(std::string const &, ReferenceFrame const *frame);
      void setModel(GLModel *);
      GLModel *model() const;

      void zoom(Real delta);
      void incAzEl(Real deltaAz, Real deltaEl);
      void roll(Real delta);
      void move(Real deltaX, Real deltaY);

      void setZoom(Real delta);
      void setCenter(Real, Real);
      void setRotation(Real, Real, Real, Real);
      void setView(GLCurrentView const *);
  };
}

#endif // _GL_RENDER_ENGINE_H

