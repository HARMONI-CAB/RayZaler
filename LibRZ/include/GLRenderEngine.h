#ifndef _GL_RENDER_ENGINE_H
#define _GL_RENDER_ENGINE_H

#include "IncrementalRotation.h"

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


  class GLRenderEngine {
      GLModel      *m_model = nullptr; // Borrowed
      GLCurrentView m_view;

    public:

      GLCurrentView *view();

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

