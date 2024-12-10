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

#include "DataProductWidget.h"
#include "AsyncDataProductRenderer.h"
#include "GUIHelpers.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>

const QRect g_infinityBox(-(INT_MAX / 2), -(INT_MAX / 2), INT_MAX, INT_MAX);

DataProductWidget::DataProductWidget(
    RZ::DataProduct *product,
    QWidget *parent)
  : QWidget{parent}, m_fontMetrics(m_tickFont)
{
  m_product = product;

  setMouseTracking(true);

  m_leftMargin   = m_fontMetrics.horizontalAdvance("X.XXX nm - ");
  m_bottomMargin = m_leftMargin;
  m_topMargin    = 2 * m_fontMetrics.height();
  m_rightMargin  = m_topMargin;
}

DataProductWidget::~DataProductWidget()
{
  m_renderThread.quit();
  m_renderThread.wait();

  if (m_product != nullptr)
    delete m_product;

  if (m_asyncRenderer != nullptr)
    m_asyncRenderer->deleteLater();
}

AsyncDataProductRenderer *
DataProductWidget::asyncRenderer()
{
  if (m_asyncRenderer == nullptr) {
    m_asyncRenderer = makeRenderer(m_product);

    m_asyncRenderer->moveToThread(&m_renderThread);

    connect(
          m_asyncRenderer,
          SIGNAL(complete(qint64, QImage *)),
          this,
          SLOT(onComplete(qint64, QImage *)));

    connect(
          m_asyncRenderer,
          SIGNAL(viewReady()),
          this,
          SLOT(onViewReady()));

    connect(
          this,
          SIGNAL(render(qint64, qreal, qreal, qreal, int, int)),
          m_asyncRenderer,
          SLOT(render(qint64, qreal, qreal, qreal, int, int)));

    connect(
          this,
          SIGNAL(makeView()),
          m_asyncRenderer,
          SLOT(makeView()));

    connect(
          this,
          SIGNAL(clearData()),
          m_asyncRenderer,
          SLOT(clearData()));

    connect(
          this,
          SIGNAL(saveData(QString)),
          m_asyncRenderer,
          SLOT(saveData(QString)));

    connect(
          m_asyncRenderer,
          SIGNAL(error(QString)),
          this,
          SLOT(onError(QString)));

    m_renderThread.start();
  }

  return m_asyncRenderer;
}

void
DataProductWidget::updateView()
{
  asyncRenderer()->discardCurrentView();
  requestRender();
  emit makeView();
}

void
DataProductWidget::addCurve(DataProductCurve &curve)
{
  m_curves.push_back(curve);
  update();
}

void
DataProductWidget::clearCurves()
{
  m_curves.clear();
  update();
}

qreal
DataProductWidget::ds() const
{
  return 1. / (m_zoom * fmin(m_viewRect.width(), m_viewRect.height()));
}

void
DataProductWidget::guessScale()
{
  auto ds = this->ds();

  auto plotWidth  = m_viewRect.width() * ds;
  auto plotHeight = m_viewRect.height() * ds;
  auto refDim     = fmin(plotWidth, plotHeight);

  int digits = static_cast<int>(ceil(log10(.5 * refDim / 10)));
  auto step  = pow(10., digits);

  m_bestCoarseStep = step;
  m_bestFineStep   = .1 * step;

  qreal val = m_bestCoarseStep;

  sensibleUnits(val, m_bestUnitDivider, m_bestUnits);
}

void
DataProductWidget::paintTicks(QPainter &p) const
{
  QPen pen;
  QColor textColor      = QColor(0, 0, 0);
  QPointF topLeft       = px2loc(m_gridTopLeft);
  QPointF bottomRight   = px2loc(m_gridBottomRight);

  qreal x0;
  qreal y0;

  p.save();

  pen.setColor(textColor);
  pen.setWidth(1);

  p.setPen(pen);

  p.drawRect(m_viewRect);

  x0 = m_bestCoarseStep * floor(topLeft.x() / m_bestCoarseStep);
  while (x0 <= bottomRight.x()) {
    QPointF p1 = loc2px(QPointF(x0, bottomRight.y()));

    if (x0 >= topLeft.x() && x0 <= bottomRight.x()) {
      if (fabs(x0 / m_bestUnitDivider) < 1e-9)
        x0 = 0;

      QString text = QString::asprintf(
            "  %+.3g ",
            x0 / m_bestUnitDivider) + " " + m_bestUnits;
      auto advance = m_fontMetrics.horizontalAdvance(text);
      QTransform t;
      t.translate(p1.x(), p1.y() + advance / 2);
      t.rotate(90);
      p.setTransform(t);
      p.drawText(g_infinityBox, Qt::AlignCenter, text);
    }

    x0 += m_bestCoarseStep;
  }

  y0 = m_bestCoarseStep * floor(bottomRight.y() / m_bestCoarseStep);
  while (y0 < topLeft.y()) {
    QPointF p1 = loc2px(QPointF(topLeft.x(), y0));

    if (y0 >= bottomRight.y() && y0 < topLeft.y()) {
      if (fabs(y0 / m_bestUnitDivider) < 1e-9)
        y0 = 0;

      QString text = QString::asprintf(
            "%+.3g ",
            y0 / m_bestUnitDivider) + " " + m_bestUnits + " ";
      auto advance = m_fontMetrics.horizontalAdvance(text);

      QTransform t;
      t.translate(p1.x() - advance / 2, p1.y());
      p.setTransform(t);
      p.drawText(g_infinityBox, Qt::AlignCenter, text);
    }

    y0 += m_bestCoarseStep;
  }

  p.restore();
}

void
DataProductWidget::paintGrid(QPainter &p) const
{
  QPen pen;
  QColor gridColor    = QColor::fromHsvF(0, 0, 0.75f);
  QPointF topLeft     = px2loc(m_gridTopLeft);
  QPointF bottomRight = px2loc(m_gridBottomRight);

  qreal x0;
  qreal y0;

  p.save();

  pen.setColor(gridColor);
  p.setPen(pen);

  // Fine grid
  pen.setWidth(1);
  gridColor.setAlphaF(.25);
  pen.setColor(gridColor);
  p.setPen(pen);

  x0 = m_bestCoarseStep * floor(topLeft.x() / m_bestCoarseStep);
  while (x0 < bottomRight.x()) {
    QPointF p1 = loc2px(QPointF(x0, topLeft.y()));
    QPointF p2 = loc2px(QPointF(x0, bottomRight.y()));

    if (x0 >= topLeft.x() && x0 < bottomRight.x())
      p.drawLine(p1, p2);

    x0 += m_bestFineStep;
  }

  y0 = m_bestCoarseStep * floor(bottomRight.y() / m_bestCoarseStep);
  while (y0 < topLeft.y()) {
    QPointF p1 = loc2px(QPointF(topLeft.x(), y0));
    QPointF p2 = loc2px(QPointF(bottomRight.x(), y0));

    if (y0 >= bottomRight.y() && y0 < topLeft.y())
      p.drawLine(p1, p2);

    y0 += m_bestFineStep;
  }

  // Coarse grid
  pen.setWidth(2);
  gridColor.setAlphaF(.5);
  pen.setColor(gridColor);
  p.setPen(pen);

  x0 = m_bestCoarseStep * floor(topLeft.x() / m_bestCoarseStep);
  while (x0 < bottomRight.x()) {
    QPointF p1 = loc2px(QPointF(x0, topLeft.y()));
    QPointF p2 = loc2px(QPointF(x0, bottomRight.y()));

    if (x0 >= topLeft.x() && x0 < bottomRight.x())
      p.drawLine(p1, p2);

    x0 += m_bestCoarseStep;
  }

  y0 = m_bestCoarseStep * floor(bottomRight.y() / m_bestCoarseStep);
  while (y0 < topLeft.y()) {
    QPointF p1 = loc2px(QPointF(topLeft.x(), y0));
    QPointF p2 = loc2px(QPointF(bottomRight.x(), y0));

    if (y0 >= bottomRight.y() && y0 < topLeft.y())
      p.drawLine(p1, p2);

    y0 += m_bestCoarseStep;
  }

  // Coarse grid (displaced)
  pen.setWidth(1);
  gridColor.setAlphaF(.5);
  pen.setColor(gridColor);
  p.setPen(pen);

  x0 = m_bestCoarseStep * (floor(topLeft.x() / m_bestCoarseStep) - .5);
  while (x0 < bottomRight.x()) {
    QPointF p1 = loc2px(QPointF(x0, topLeft.y()));
    QPointF p2 = loc2px(QPointF(x0, bottomRight.y()));

    if (x0 >= topLeft.x() && x0 < bottomRight.x())
      p.drawLine(p1, p2);

    x0 += m_bestCoarseStep;
  }

  y0 = m_bestCoarseStep * (floor(bottomRight.y() / m_bestCoarseStep) - .5);
  while (y0 < topLeft.y()) {
    QPointF p1 = loc2px(QPointF(topLeft.x(), y0));
    QPointF p2 = loc2px(QPointF(bottomRight.x(), y0));

    if (y0 >= bottomRight.y() && y0 < topLeft.y())
      p.drawLine(p1, p2);

    y0 += m_bestCoarseStep;
  }

  p.restore();
}

void
DataProductWidget::paintLastRender(QPainter &painter)
{
  QPointF imgLocation;

  paintTicks(painter);
  paintGrid(painter);
  paintCurves(painter);

  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

  // When there is a change in both the zoom and the location, we must take into
  // account that:
  //
  // m_currPos is the displacement of the center of the image, in pixels
  // currOrigin is the location of the center of the image wrt the topLeft corner
  //
  // The last render had its center at m_renderPos and the topLeft corner
  // was in m_renderOrigin. In order to understand ho much one image was
  // displaced wrt the other, we need to convert both to world (plot) coordinates
  // and then back to current screen coordinates. This is:
  //
  // DeltaX  = m_currPos * ds() - m_renderPos * renderDs
  // DeltaPx = DeltaX / ds() = m_currPos - m_renderPos * renderDs / ds()
  //
  // Now, here's the thing. If we just compute m_currPos - m_renderPos, we make
  // sure the image is displaced wrt its topLeft corner. But we do not want that,
  // we want it to be displaced w.r.t its center. The topLeft corner of the
  // render image is, in former world coordinates, at:
  //
  // renderCorner = -renderDs * (width/2, height/2)
  //
  // And the current one is at:
  //
  // currentCorner = -ds() * (width/2, height/2)
  //
  // The difference in pixels is therefore:
  //
  // DeltaPx += (currentCorner - renderConer) / ds() =
  //         += -(width/2, height/2) + (width/2, height/2) * renderDs / ds()
  //         += - (width/2, height) * (renderDs / ds()  - 1)

  QImage *ptrImage = &m_image;
  QImage  image;
  if (fabs(m_zoom - m_lastRender.zoom) > 1e-9) {
    qreal dRsds = m_lastRender.ds / ds();
    qreal scale = m_zoom / m_lastRender.zoom;
    imgLocation = m_currPos - m_lastRender.pos * dRsds;

    imgLocation -= .5 * QPointF(m_image.width(), m_image.height()) * (dRsds - 1.);

    image = m_image.scaled(
          static_cast<int>(scale * m_image.width()),
          static_cast<int>(scale * m_image.height()));
    ptrImage = &image;
  } else {
    imgLocation = m_currPos - m_lastRender.pos;
  }

  QRectF imageRect = m_viewRect;
  imageRect.moveTo(-imgLocation.x(), -imgLocation.y());
  painter.drawImage(m_viewRect, *ptrImage, imageRect);
}

void
DataProductWidget::paintBusyMessage(QPainter &painter)
{
  QPointF p1 = m_gridTopLeft + QPointF(m_viewRect.width() / 2, m_viewRect.height() / 2);
  QString text = "Building view...";
  QTransform t;

  painter.fillRect(m_viewRect, QColor(0xbf, 0xbf, 0xbf, 0xff));

  painter.save();
  t.translate(p1.x(), p1.y());
  painter.setTransform(t);
  painter.drawText(g_infinityBox, Qt::AlignCenter, text);
  painter.restore();

  QPen pen(QColor(0, 0, 0));
  pen.setWidth(1);
  painter.setPen(pen);
  painter.drawRect(m_viewRect);
}

void
DataProductWidget::paintLabels(QPainter &painter)
{
  QPointF titlePos =
      m_gridTopLeft
      + QPointF(m_viewRect.width() / 2, -m_topMargin / 2);
  QTransform t;

  painter.save();
  t.translate(titlePos.x(), titlePos.y());
  painter.setTransform(t);
  painter.drawText(
        g_infinityBox,
        Qt::AlignCenter,
        QString::fromStdString(m_product->productName()));
  painter.restore();
}

void
DataProductWidget::paintCurves(QPainter &painter)
{
  QPen pen;

  painter.save();

  painter.setClipRect(m_viewRect);

  for (auto &curve : m_curves) {
    std::vector<QPointF> points;
    points.resize(curve.xydata.size());

    pen.setColor(curve.color);
    pen.setWidth(curve.width);

    painter.setPen(pen);

    for (size_t i = 0; i < curve.xydata.size(); ++i)
      points[i] = loc2px(curve.xydata[i]);

    if (curve.closed && !points.empty())
      points.push_back(points[0]);

    painter.drawPolyline(points.data(), points.size());
  }

  painter.restore();
}

void
DataProductWidget::paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  guessScale();

  painter.fillRect(rect(), QColor(255, 255, 255));

  if (m_needsNewView) {
    QImage image(
          static_cast<int>(m_viewRect.width()),
          static_cast<int>(m_viewRect.height()),
          QImage::Format_ARGB32);

    if (asyncRenderer()->haveView()) {
      image.fill(0);
      asyncRenderer()->renderToImage(image, m_zoom, m_x0, m_y0);
    } else {
      image.fill(0xffbfbfbf);
    }

    m_image        = image;
    setCurrentRenderAsReference();
    m_needsNewView = false;
  }

  if (asyncRenderer()->haveView())
    paintLastRender(painter);
  else
    paintBusyMessage(painter);

  paintLabels(painter);

  painter.end();
}

void
DataProductWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::MouseButton::MiddleButton) {
    // Drag start
    m_moveRefPos = event->position();
    m_haveRefPos = true;
  }
}

void
DataProductWidget::resetZoom()
{
  m_zoom = m_resetZoom;
  m_currPos = QPointF();
  m_x0 = m_resetX0;
  m_y0 = m_resetY0;
  emit viewChanged();
  requestRender();
  update();
}

void
DataProductWidget::setResetZoom(qreal zoom, qreal x0, qreal y0)
{
  m_resetZoom = zoom;
  m_resetX0   = x0;
  m_resetY0   = y0;
}

void
DataProductWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if (m_interactive) {
    if (event->button() == Qt::MouseButton::RightButton)
      resetZoom();

    // Drag end, if any
    m_haveRefPos  = false;
    m_haveLastPos = false;
    requestRender();
  }
}

QPointF
DataProductWidget::px2loc(QPointF const &px) const
{
  auto result = (px - .5 * QPointF(m_viewRect.width(), m_viewRect.height()) - m_gridTopLeft - m_currPos) * ds();
  result.setY(-result.y());

  return result;
}

QPointF
DataProductWidget::loc2px(QPointF const &input) const
{
  QPointF loc = input;

  loc.setY(-loc.y());

  return loc / ds() + .5 * QPointF(m_viewRect.width(), m_viewRect.height()) + m_currPos + m_gridTopLeft;
}

void
DataProductWidget::resizeEvent(QResizeEvent *)
{
  qreal oldDim = fmin(m_viewRect.width(), m_viewRect.height());

  m_gridTopLeft = QPointF(m_leftMargin, m_topMargin);
  m_gridBottomRight = QPointF(
        width() - m_rightMargin - 1,
        height() - m_bottomMargin - 1);

  m_viewRect = QRectF(m_gridTopLeft, m_gridBottomRight);


  qreal currDim = fmin(m_viewRect.width(), m_viewRect.height());

  if (m_firstResize) {
    oldDim = currDim;
    m_firstResize = false;
    updateView();
  }

  m_zoom            *= oldDim / currDim;
  m_lastRender.zoom *= oldDim / currDim;

  auto ds   = this->ds();
  m_currPos = -QPointF(m_x0 / ds, -m_y0 / ds);

  update();
  requestRender();
}

void
DataProductWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (m_interactive) {
    if (m_haveRefPos) {
      if (m_haveLastPos) {

        // Dragging
        auto delta = event->position() - m_moveLastPos;
        m_currPos += delta;
      }

      auto ds = this->ds();
      m_moveLastPos = event->position();
      m_haveLastPos = true;
      m_x0 = -m_currPos.x() * ds;
      m_y0 = +m_currPos.y() * ds;
      requestRender();
      emit viewChanged();
      update();
    }
  }
}

//
// When we zoom around an arbitrary pixel (i, j) we want that the location
// of that pixel to remain the same after the zoom. This is:
// xy = px2loc(px)
// loc2px(xy) = px
//
// Therefore:
// xy = (px - .5 * size - currPos) * ds
// px = (xy / ds' + .5 * size + currPos')
//
// xy = ((xy / ds' + .5 * size + currPos') - .5 * size - currPos) * ds
// xy = ((xy / ds' + currPos' - currPos) * ds
//
// xy / ds = xy / ds' + currPos' - currPos
// xy (1 / ds - 1 / ds') = currPos' - currPos
//
// Therefore:
// currPos' = xy (1 / ds - 1 / ds') + currPos
//

void
DataProductWidget::wheelEvent(QWheelEvent *event)
{
  if (m_interactive) {
    // qreal prevZoom = m_zoom;
    auto dsOld  = ds();
    auto xy     = px2loc(event->position());
    m_zoom     *= exp(event->angleDelta().y() / 1200.);

    auto dsNew  = ds();

    auto delta = xy * (1 / dsOld - 1 / dsNew);
    delta.setY(-delta.y());

    m_currPos  += delta;

    m_x0 = -m_currPos.x() * dsNew;
    m_y0 = +m_currPos.y() * dsNew;

    //m_needsNewView = true;

    emit viewChanged();
    requestRender();
    update();
  }
}

void
DataProductWidget::requestRender()
{
  // The async renderer reports a "huge" data product, i.e. a data product
  // that is hard to render. Go async and paint when it is done.
  if (asyncRenderer()->isBig()) {
    ++m_reqId;

    RenderInfo info;
    info.pos  = m_currPos;
    info.ds   = ds();
    info.zoom = m_zoom;

    m_renderHistory[m_reqId] = info;

    asyncRenderer()->setLastReqId(m_reqId);
    emit render(
          m_reqId,
          m_zoom,
          m_x0,
          m_y0,
          m_viewRect.width(),
          m_viewRect.height());
  } else {
    m_needsNewView = true;
    update();
  }
}

void
DataProductWidget::setCurrentRenderAsReference()
{
  m_lastRender.pos    = m_currPos;
  m_lastRender.zoom   = m_zoom;
  m_lastRender.ds     = ds();
  m_needsNewView      = false;
}

void
DataProductWidget::onViewReady()
{
  requestRender();
}

void
DataProductWidget::onComplete(qint64 reqId, QImage *image)
{
  if (m_renderHistory.contains(reqId)) {
    m_image = *image;
    asyncRenderer()->returnImage(image);
    m_lastRender = m_renderHistory[reqId];
    m_renderHistory.remove(reqId);
    update();
  }
}

void
DataProductWidget::onError(QString error)
{
  QMessageBox::critical(this, "Data product", error);
}
