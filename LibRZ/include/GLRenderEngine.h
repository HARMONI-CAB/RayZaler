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

#ifndef _GL_RENDER_ENGINE_H
#define _GL_RENDER_ENGINE_H

#include "IncrementalRotation.h"
#include <GLHelpers.h>
#include <list>

namespace RZ {
  class GLModel;
  class Element;

  struct GLCurrentView {
    Real                  zoomLevel = 1;
    Real                  center[2] = {0, 0};
    Real                  width, height;
    IncrementalRotation   rotation;
    IncrementalRotation   screenRotation;

    void zoom(Real delta);
    void incAzEl(Real deltaAz, Real deltaEl);
    void roll(Real delta);
    void move(Real deltaX, Real deltaY);

    void setScreenGeom(Real width, Real height);
    void setZoom(Real delta);
    void setCenter(Real, Real);
    void setRotation(Real, Real, Real, Real);
    void setRotation(Vec3 const &, Real);
    void setRotation(Matrix3 const &);
    void setScreenRotation(IncrementalRotation const &);
    void rotateRelative(Vec3 const &, Real);
    void configureViewPort(unsigned int width, unsigned int height) const;
    void configureOrientation(bool translate = true) const;

    void screenToWorld(Real &wX, Real &wY, Real sX, Real sY) const;
    void worldToScreen(Real &sX, Real &sY, Real wX, Real wY) const;

    bool screenToFrame(Vec3 &, ReferenceFrame const &, Real sX, Real sY) const;
    bool worldToFrame(Vec3 &, ReferenceFrame const &, Real wX, Real wY) const;

    void frameToWorld(Real &wX, Real &wY, ReferenceFrame const &, Vec3 const &) const;
    void frameToScreen(Real &sX, Real &sY, ReferenceFrame const &, Vec3 const &) const;

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
    const ReferenceFrame *frame = nullptr; // Borrowed
    GLHelperGrid grid;
  };

  class GLRenderEngine {
      GLModel               *m_model = nullptr; // Borrowed
      GLCurrentView          m_view;
      GLReferenceFrame       m_glAxes;
      std::list<GLFrameGrid> m_grids;

    protected:
      void setOrientationAndCenter(RZ::Matrix3 const &R, RZ::Vec3 const &O);
      void drawCornerAxes();
      void drawGrids();
      
    public:
      GLCurrentView *view();
      GLFrameGrid *addGrid(std::string const &, ReferenceFrame const *frame);
      
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

