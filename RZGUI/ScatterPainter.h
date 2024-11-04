#ifndef _SCATTER_PAINTER_H
#define _SCATTER_PAINTER_H

#include "ScatterTree.h"
#include <QPainter>
#include <QImage>

//
// The way we treat zoom is as follows: at zoom N, distances equal to 1
// are treated as N times the width of the device.
//

class ScatterPainter : public ScatterTreeRenderer {
  QPainter  m_painter;
  QImage   *m_image = nullptr;
  uint32_t *m_imgData;

  int       m_width;
  int       m_height;
  int       m_stride;
  double    m_zoom = 1;
  double    m_x0   = 0;
  double    m_y0   = 0;
  double    m_dx   = 0;

  int x2px(double x) const;
  int y2py(double x) const;

  double px2x(int) const;
  double px2y(int) const;

  inline void
  pset(int x, int y, uint32_t color) {
    m_imgData[x + y * m_stride] = color;
  }

public:
  ScatterPainter(QImage &, double zoom = 1, double x0 = 0, double y0 = 0);

  virtual ScatterVec resolution() const override;
  virtual ScatterVec topLeft() const override;
  virtual ScatterVec bottomRight() const override;
  virtual void render(int x, int y, unsigned int count) override;
};

#endif // _SCATTER_PAINTER_H

