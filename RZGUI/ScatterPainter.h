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

#ifndef _SCATTER_PAINTER_H
#define _SCATTER_PAINTER_H

#include <DataProducts/ScatterTree.h>
#include <QPainter>
#include <QImage>

//
// The way we treat zoom is as follows: at zoom N, distances equal to 1
// are treated as N times the width of the device.
//

class ScatterPainter : public RZ::ScatterTreeRenderer {
  QPainter  m_painter;
  QImage   *m_image = nullptr;
  uint32_t *m_imgData;
  uint32_t  m_id = 0xff0000ff;

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

  virtual void setId(uint32_t id) override;
  virtual RZ::ScatterVec resolution() const override;
  virtual RZ::ScatterVec topLeft() const override;
  virtual RZ::ScatterVec bottomRight() const override;
  virtual void render(int x, int y, unsigned int count) override;
};

#endif // _SCATTER_PAINTER_H

