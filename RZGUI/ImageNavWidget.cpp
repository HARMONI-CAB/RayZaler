#include "ImageNavWidget.h"
#include <Detector.h>
#include <QPainter>
#include <QPixmap>
#include <QImage>

ImageNavWidget::ImageNavWidget(QWidget *parent)
  : QWidget{parent}
{
  setMouseTracking(true);
}


void
ImageNavWidget::recalcImage()
{
  if (m_detector == nullptr) {
    m_image = QImage();
  } else {
    uint32_t max = 0;
    uint32_t min = 0xffffffff;
    unsigned int width  = m_detector->cols();
    unsigned int height = m_detector->rows();
    unsigned int stride = m_detector->stride();
    unsigned int allocation = width * stride;
    m_asBytes.resize(allocation);

    const uint32_t *photons = m_detector->data();

    if (m_autoscale) {
      for (unsigned int j = 0; j < height; ++j) {
        for (unsigned int i = 0; i < width; ++i) {
          auto val = photons[i + j * stride];

          if (val > max)
              max = val;
          if (val < min)
            min = val;
        }
      }

      m_sane_min = m_arr_min = min;
      m_sane_max = m_arr_max = max;
    }



    // In log scale, min is black and max is white
    if (m_logScale) {
      qreal rangeInv = 1. / (m_arr_max - m_arr_min + 1);
      qreal black = log(rangeInv);
      qreal white = 1;
      qreal kInv = 255. / (white - black);


      for (unsigned int j = 0; j < height; ++j) {
        for (unsigned int i = 0; i < width; ++i) {
          auto val = log(rangeInv * (photons[i + j * stride] - m_arr_min + 1));

          m_asBytes[i + j * width] =
              static_cast<uint8_t>(
                qBound(
                  static_cast<uint32_t>(0),
                  static_cast<uint32_t>(kInv * (val - black)),
                  static_cast<uint32_t>(255)));
        }
      }
    } else {
      qreal kInv = 255. / (m_arr_max - m_arr_min);
      for (unsigned int j = 0; j < height; ++j) {
        for (unsigned int i = 0; i < width; ++i) {
          auto val = photons[i + j * stride];

          m_asBytes[i + j * width] =
              static_cast<uint8_t>(
                qBound(
                  static_cast<uint32_t>(0),
                  static_cast<uint32_t>(kInv * (val - m_arr_min)),
                  static_cast<uint32_t>(255)));
        }
      }
    }


    m_image = QImage(
          static_cast<uchar *>(m_asBytes.data()),
          width,
          height,
          QImage::Format::Format_Grayscale8);
  }
}

QPoint
ImageNavWidget::px2img(QPoint p) const
{
  auto imgcenter = px2imgcenter(p);
  auto center = QPoint(m_image.width(), m_image.height()) / 2;
  return imgcenter + center;
}

QPoint
ImageNavWidget::img2px(QPoint xy) const
{
  auto center = QPoint(m_image.width(), m_image.height()) / 2;
  return imgcenter2px(xy - center);
}

QPoint
ImageNavWidget::px2imgcenter(QPoint p) const
{
  return
        (p
         - QPoint(width(), height()) / 2
         - m_currPos.toPoint()) / m_zoom;
}

QPoint
ImageNavWidget::imgcenter2px(QPoint xy) const
{
  return xy * m_zoom + m_currPos.toPoint() + QPoint(width(), height()) / 2;
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
    uint32_t max = 0;
    uint32_t min = 0xffffffff;

    unsigned int width  = m_detector->cols();
    unsigned int height = m_detector->rows();
    unsigned int stride = m_detector->stride();
    const uint32_t *photons = m_detector->data();

    for (unsigned int j = 0; j < height; ++j) {
      for (unsigned int i = 0; i < width; ++i) {
        auto val = photons[i + j * stride];

        if (val > max)
            max = val;
        if (val < min)
          min = val;
      }
    }

    m_sane_min = min;
    m_sane_max = max;
    m_ratio = static_cast<qreal>(height) / static_cast<qreal>(min);
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

void
ImageNavWidget::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  if (!m_image.isNull()) {
    auto target_width  = m_image.width()  * m_zoom;
    auto target_height = m_image.height() * m_zoom;
    auto target        = imgcenter2px(
          QPoint(-m_image.width() / 2, -m_image.height() / 2));

    painter.drawPixmap(
          QRect(
            target,
            QSize(
              static_cast<int>(target_width),
              static_cast<int>(target_height))),
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
        auto delta = event->position() - m_move_last_pos;
        m_currPos += delta;
      }

      m_move_last_pos = event->position();
      m_have_last_pos = true;
      emit viewChanged();
      update();
    } else {
      emit mouseMoved(px2imgcenter(event->position().toPoint()));
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
    auto xy = px2imgcenter(event->position().toPoint());
    m_zoom    *= exp(event->angleDelta().y() / 1200.);
    m_currPos += xy * (prev_zoom - m_zoom);

    emit viewChanged();
    update();
  }
}
