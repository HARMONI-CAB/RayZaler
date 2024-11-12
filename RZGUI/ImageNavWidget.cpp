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

#include "ImageNavWidget.h"
#include <Detector.h>
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include "GUIHelpers.h"
#include "YIQ.h"

#define LOG_PHOT_MIN   1
#define LOG_ENERGY_MIN 1e-14

ImageNavWidget::ImageNavWidget(QWidget *parent)
  : QWidget{parent}
{
  setMouseTracking(true);
}

inline qreal
ImageNavWidget::pixelValue(unsigned p)
{
  if (m_showPhotons) {
    const uint32_t *photons = m_detector->data();
    return photons[p]; 
  } else {
    const RZ::Complex A = m_detector->amplitude()[p];
    RZ::Real E = std::real(A * std::conj(A));
    return E;
  }
}

inline qreal
ImageNavWidget::pixelPhase(unsigned p)
{
  const RZ::Complex A = m_detector->amplitude()[p];
  return std::arg(A);
}

inline void
ImageNavWidget::psetRGB(unsigned p, qreal val, qreal phase)
{
  phase = fmod(phase, 2 * M_PI);
  if (phase < 0)
    phase += 2 * M_PI;

  unsigned index = static_cast<unsigned>(floor(phase / (2 * M_PI) * 1024)) % 1024;

  QColor phaseColor = yiqTable[index];

  uint8_t red   = pixBound(val * phaseColor.red());
  uint8_t green = pixBound(val * phaseColor.green());
  uint8_t blue  = pixBound(val * phaseColor.blue());

  m_asBytes[3 * p + 0] = red;
  m_asBytes[3 * p + 1] = green;
  m_asBytes[3 * p + 2] = blue;
}

void
ImageNavWidget::recalcImage()
{
  if (m_detector == nullptr) {
    m_image = QImage();
  } else {
    unsigned int width  = m_detector->cols();
    unsigned int height = m_detector->rows();
    unsigned int stride = m_detector->stride();
    unsigned int bpp    = m_showPhase ? 3 : 1;
    unsigned int allocation = bpp * stride * height;
    m_asBytes.resize(allocation);

    if (m_autoscale) {
      m_sane_max = m_arr_max = m_showPhotons 
        ? m_detector->maxCounts()
        : m_detector->maxEnergy();

      qreal min = m_arr_max;

      for (unsigned j = 0; j < height; ++j)
        for (unsigned i = 0; i < width; ++i)
          if (pixelValue(i + j * stride) < min)
            min = pixelValue(i + j * stride);
      
      if (min > m_sane_min || !m_logScale)
        m_sane_min = m_arr_min = min;
    }

    // In log scale, min is black and max is white
    if (m_logScale) {
      qreal minVal   = m_showPhotons ? LOG_PHOT_MIN : LOG_ENERGY_MIN;
      qreal rangeInv = 1. / (m_arr_max - m_arr_min + minVal);
      qreal black = log((m_arr_min + minVal) * rangeInv);
      qreal white = 1;
      qreal kInv = 1. / (white - black);

      for (unsigned j = 0; j < height; ++j) {
        for (unsigned i = 0; i < width; ++i) {
          unsigned int p = i + j * stride;
          auto val = log(rangeInv * (pixelValue(p) - m_arr_min + minVal));

          if (m_showPhase)
            psetRGB(p, kInv * (val - black), pixelPhase(p));
          else
            m_asBytes[p] = pixBound(255 * kInv * (val - black));
        }
      }
    } else {
      qreal kInv = 1. / (m_arr_max - m_arr_min);
      for (unsigned j = 0; j < height; ++j) {
        for (unsigned i = 0; i < width; ++i) {
          unsigned int p = i + j * stride;

          auto val = pixelValue(p);

          if (m_showPhase)
            psetRGB(p, kInv * (val - m_arr_min), pixelPhase(p));
          else
            m_asBytes[p] = pixBound(255 * kInv * (val - m_arr_min));
        }
      }
    }

    if (m_showPhase) {
      m_image = QImage(
            static_cast<uchar *>(m_asBytes.data()),
            SCAST(int, width),
            SCAST(int, height),
            bpp * stride,
            QImage::Format::Format_RGB888);
    } else {
      m_image = QImage(
            static_cast<uchar *>(m_asBytes.data()),
            SCAST(int, width),
            SCAST(int, height),
            stride,
            QImage::Format::Format_Grayscale8);
    }
  }
}

QPoint
ImageNavWidget::px2img(QPoint p) const
{
  return px2img(QPointF(p)).toPoint();
}

QPoint
ImageNavWidget::img2px(QPoint xy) const
{
  return img2px(QPointF(xy)).toPoint();
}

QPointF
ImageNavWidget::px2img(QPointF p) const
{
  auto imgcenter = px2imgcenter(p);
  auto center = QPointF(m_image.width(), m_image.height()) * .5;
  return imgcenter + center;
}

QPointF
ImageNavWidget::img2px(QPointF xy) const
{
  auto center = QPointF(m_image.width(), m_image.height()) * .5;
  return imgcenter2px(xy - center);
}

QPoint
ImageNavWidget::px2imgcenter(QPoint p) const
{
  return px2imgcenter(QPointF(p)).toPoint();
}

QPointF
ImageNavWidget::px2imgcenter(QPointF p) const
{
  return (p - QPointF(width(), height()) * .5 - m_currPos) / m_zoom;
}

QPoint
ImageNavWidget::imgcenter2px(QPoint xy) const
{
  return imgcenter2px(QPointF(xy)).toPoint();
}

QPointF
ImageNavWidget::imgcenter2px(QPointF xy) const
{
  return xy * m_zoom + m_currPos + QPointF(width(), height()) * .5;
}

QPointF
ImageNavWidget::getSelection() const
{
  return m_currSel;
}

void
ImageNavWidget::setInteractive(bool interactive)
{
  m_interactive = interactive;
  setMouseTracking(interactive);
}
void
ImageNavWidget::setImageLimits(qreal min, qreal max)
{
  m_arr_min = min;
  m_arr_max = max;
  recalcImage();
  update();
}

void
ImageNavWidget::setAutoScale(bool autoscale)
{
  if (m_autoscale != autoscale) {
    m_autoscale = autoscale;
    if (autoscale)
      setImageLimits(m_sane_min, m_sane_max);
  }
}

void
ImageNavWidget::setDetector(RZ::Detector *det)
{
  m_detector = det;

  if (det != nullptr) {
    unsigned int width  = m_detector->cols();
    unsigned int height = m_detector->rows();

    m_sane_min = 0;
    m_sane_max = m_showPhotons 
      ? m_detector->maxCounts()
      : m_detector->maxEnergy();
    m_ratio = static_cast<qreal>(height) / static_cast<qreal>(width);
  } else {
    m_sane_min = 0;
    m_sane_max = 1;
    m_ratio    = 1;
  }

  recalcImage();
  emit viewChanged();
  update();
}

void
ImageNavWidget::setZoom(qreal zoom)
{
  m_preferredZoom = zoom;
  m_zoom          = zoom;
  emit viewChanged();
  update();
}

void
ImageNavWidget::setLogScale(bool scale)
{
  if (m_logScale != scale) {
    m_logScale = scale;
    recalcImage();
    update();
  }
}

void
ImageNavWidget::resetZoom()
{
  m_zoom = m_preferredZoom;
  m_currPos = QPointF();
  emit viewChanged();
  update();
}

void
ImageNavWidget::zoomToPoint(QPointF const & xy)
{
  m_currPos = -xy * m_zoom;
  emit viewChanged();
  update();
}

void
ImageNavWidget::setShowPhotons(bool show)
{
  if (show != m_showPhotons) {
    m_showPhotons = show;
    recalcImage();
    update();
  }
}

void
ImageNavWidget::setShowPhase(bool show)
{
  if (show != m_showPhase) {
    m_showPhase = show;
    recalcImage();
    update();
  }
}

void
ImageNavWidget::setShowGrid(bool show)
{
  if (show != m_showGrid) {
    m_showGrid = show;
    update();
  }
}

qreal
ImageNavWidget::imgMin() const
{
  return m_arr_min;
}

qreal
ImageNavWidget::imgMax() const
{
  return m_arr_max;
}

void
ImageNavWidget::setSelection(QPoint const &xy)
{
  int sel_x = qBound(0, xy.x(), m_image.width() - 1);
  int sel_y = qBound(0, xy.y(), m_image.height() - 1);
  QPoint newSel(sel_x, sel_y);

  if (newSel != m_currSel) {
    m_currSel = newSel;
    update();
  }
}

void
ImageNavWidget::updateSelectionFromEvent(QMouseEvent *event, bool last)
{
  auto xy = px2img(event->pos());
  setSelection(xy);
  emit selChanged(last);
  update();
}

QSize
ImageNavWidget::imageSize() const
{
  return m_image.size();
}

QSizeF
ImageNavWidget::viewSize() const
{
  return QSizeF(size()) / m_zoom;
}

QPointF
ImageNavWidget::currPoint() const
{
  return m_currPos;
}

qreal
ImageNavWidget::zoom() const
{
  return m_zoom;
}

void
ImageNavWidget::setCurrPoint(QPointF const &p)
{
  m_currPos = p;
  update();
}

QPoint
ImageNavWidget::boundToRect(QPoint const &p, QRect const &rect)
{
  return QPoint(
    qBound(rect.x(), p.x(), rect.x() + rect.width()),
    qBound(rect.y(), p.y(), rect.y() + rect.height()));
}

void
ImageNavWidget::paintGrid(QPainter &painter) const
{
  auto x0y0 = boundToRect(px2img(QPoint(0, 0)), m_image.rect());
  auto xnyn = boundToRect(px2img(QPoint(width(), height())), m_image.rect());

  int x0 = x0y0.x();
  int y0 = x0y0.y();
  int xn = xnyn.x();
  int yn = xnyn.y();

  painter.save();

  QPen pen;
  QColor color(Qt::red);
  auto alpha = qBound(0.f, m_zoom * m_zoom / 64.f, 1.f);

  color.setAlphaF(alpha);

  pen.setWidth(1);
  pen.setColor(color);
  painter.setPen(pen);

  for (int j = y0; j <= yn; ++j) {
    auto v0 = img2px(QPoint(qBound(0, x0 - 1, m_image.width()), j));
    auto vn = img2px(QPoint(qBound(0, xn + 1, m_image.width()), j));

    painter.drawLine(v0, vn);
  }

  for (int i = x0; i <= xn; ++i) {
    auto h0 = img2px(QPoint(i, qBound(0, y0 - 1, m_image.height())));
    auto hn = img2px(QPoint(i, qBound(0, yn + 1, m_image.height())));
    painter.drawLine(h0, hn);
  }

  painter.restore();
}

void
ImageNavWidget::paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  if (!m_image.isNull()) {
    auto target_width  = static_cast<int>(ceil(m_image.width()  * m_zoom));
    auto target_height = static_cast<int>(ceil(m_image.height() * m_zoom));
    auto targetF        = imgcenter2px(
          QPointF(-m_image.width() * .5, -m_image.height() * .5));

    painter.drawPixmap(
          QRectF(
            targetF,
            QSize(target_width, target_height)).toRect(),
          QPixmap::fromImage(m_image));

    auto px_sel_start = img2px(m_currSel);
    auto px_sel_end   = img2px(m_currSel);

    if (px_sel_start.x() < width()
        && px_sel_start.y() < height()
        && px_sel_end.x() >= 0
        && px_sel_end.y() >= 0) {
      painter.setPen(QColor(0, 255, 0));
      painter.drawRect(QRectF(px_sel_start, px_sel_end));
    }

    if (m_zoom > 2 && m_showGrid)
      paintGrid(painter);
  } else {
    QBrush brush;
    brush.setColor(QColor(0, 0, 0));
    painter.fillRect(rect(), brush);
  }

  painter.end();
}

void
ImageNavWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::MouseButton::MiddleButton) {
    // Drag start
    m_move_ref_pos = event->position();
    m_have_ref_pos = true;
  }

  if (event->button() == Qt::MouseButton::LeftButton
      && rect().contains(event->position().toPoint())) {
    m_movingSelection = true;
    updateSelectionFromEvent(event);
  }
}

void
ImageNavWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if (m_interactive) {
    if (event->button() == Qt::MouseButton::RightButton)
      resetZoom();

    // Drag end, if any
    m_have_ref_pos  = false;
    m_have_last_pos = false;

    if (event->button() == Qt::MouseButton::LeftButton) {
      if (m_movingSelection)
        updateSelectionFromEvent(event);
      m_movingSelection = false;
    }
  }
}

void
ImageNavWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (m_interactive) {
    if (m_have_ref_pos) {
      if (m_have_last_pos) {

        // Dragging
        auto delta = event->position() - m_move_last_pos;
        m_currPos += delta;
      }

      m_move_last_pos = event->position();
      m_have_last_pos = true;
      emit viewChanged();
      update();
    } else {
      emit mouseMoved(px2imgcenter(event->position()));
    }

    if (m_movingSelection)
      updateSelectionFromEvent(event, false);
  }
}

void
ImageNavWidget::wheelEvent(QWheelEvent *event)
{
  if (m_interactive) {
    qreal prev_zoom = m_zoom;
    auto xy = px2imgcenter(event->position());
    m_zoom    *= exp(event->angleDelta().y() / 1200.);
    m_currPos += xy * (prev_zoom - m_zoom);

    emit viewChanged();
    update();
  }
}
