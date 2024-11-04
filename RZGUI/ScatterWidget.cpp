#include "ScatterWidget.h"
#include "ScatterTree.h"
#include "ScatterPainter.h"
#include "ScatterAsyncRenderer.h"
#include "GUIHelpers.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <sys/time.h>

ScatterWidget::ScatterWidget(QWidget *parent)
  : QWidget{parent}, m_fontMetrics(m_tickFont)
{
  m_tree = new ScatterTree;

  setMouseTracking(true);

  printf("Generating a bunch of random numbers...\n");

  unsigned int i;

  for (i = 0; i < 120600; ++i) {
//qreal R = 1e-2 * (double) rand() / (double) RAND_MAX + .5;
    qreal R = 1e-1 + 1e-1 * (double) rand() / (double) RAND_MAX;
    qreal theta = 2 * M_PI * (double) rand() / (double) RAND_MAX;

    m_tree->push(R * cos(theta), R * sin(theta));
  }

  m_leftMargin   = m_fontMetrics.horizontalAdvance("X.XXX nm - ");
  m_bottomMargin = m_leftMargin;
  m_topMargin    = 2 * m_fontMetrics.height();
  m_rightMargin  = m_topMargin;

  m_asyncRenderer = new ScatterAsyncRenderer(nullptr, m_tree);

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

  m_renderThread.start();
}

ScatterWidget::~ScatterWidget()
{
  m_renderThread.quit();
  m_renderThread.wait();

  if (m_tree != nullptr)
    delete m_tree;

  if (m_asyncRenderer != nullptr)
    m_asyncRenderer->deleteLater();
}

void
ScatterWidget::triggerNewView()
{
  printf("Trigger new view!\n");

  m_asyncRenderer->discardCurrentView();
  requestRender();
  emit makeView();
}

qreal
ScatterWidget::ds() const
{
  return 1. / (m_zoom * m_viewRect.width());
}

void
ScatterWidget::guessScale()
{
  auto ds = this->ds();

  auto plotWidth  = m_viewRect.width() * ds;
  auto plotHeight = m_viewRect.height() * ds;
  auto refDim     = fmin(plotWidth, plotHeight);

  int digits = static_cast<int>(ceil(log10(refDim / 10)));
  auto step  = pow(10., digits);

  m_bestCoarseStep = step;
  m_bestFineStep   = .1 * step;

  qreal val = m_bestCoarseStep;

  sensibleUnits(val, m_bestUnitDivider, m_bestUnits);
}

void
ScatterWidget::paintTicks(QPainter &p) const
{
  QPen pen;
  QColor textColor      = QColor(0, 0, 0);
  QPointF topLeft       = px2loc(m_gridTopLeft);
  QPointF bottomRight   = px2loc(m_gridBottomRight);

  auto infinityBox = QRect(-(INT_MAX / 2), -(INT_MAX / 2), INT_MAX, INT_MAX);
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
      p.drawText(infinityBox, Qt::AlignCenter, text);
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
      p.drawText(infinityBox, Qt::AlignCenter, text);
    }

    y0 += m_bestCoarseStep;
  }

  p.restore();
}

void
ScatterWidget::paintGrid(QPainter &p) const
{
  QPen pen;
  QColor gridColor    = QColor(128, 128, 128);
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
ScatterWidget::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  QPointF imgLocation;

  guessScale();

  painter.fillRect(rect(), QColor(255, 255, 255));

  if (m_needsNewView) {
    QImage image(
          m_viewRect.width(),
          m_viewRect.height(),
          QImage::Format_ARGB32);

    if (m_asyncRenderer->haveView()) {
      ScatterPainter renderer(image, m_zoom, m_x0, m_y0);
      image.fill(0);
      m_tree->render(&renderer);
    } else {
      image.fill(0xffbfbfbf);
    }

    m_image        = image;
    setCurrentRenderAsReference();
    m_needsNewView = false;
  }

  paintGrid(painter);
  paintTicks(painter);

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
  if (fabs(m_zoom - m_renderZoom) > 1e-9) {
    qreal dRsds = m_renderDs / ds();
    qreal scale = m_zoom / m_renderZoom;
    imgLocation = m_currPos - m_renderPos * dRsds;

    imgLocation -= .5 * QPointF(m_image.width(), m_image.height()) * (dRsds - 1.);

    image = m_image.scaled(
          static_cast<int>(scale * m_image.width()),
          static_cast<int>(scale * m_image.height()));
    ptrImage = &image;
  } else {
    imgLocation = m_currPos - m_renderPos;
  }

  QRectF imageRect = m_viewRect;

  imageRect.moveTo(-imgLocation.x(), -imgLocation.y());

  painter.drawImage(m_viewRect, *ptrImage, imageRect);

  painter.end();
}

void
ScatterWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::MouseButton::MiddleButton) {
    // Drag start
    m_moveRefPos = event->position();
    m_haveRefPos = true;
  }
}

void
ScatterWidget::resetZoom()
{
  m_zoom = 1.;
  m_currPos = QPointF();
  m_x0 = m_y0 = 0;
  emit viewChanged();
  requestRender();
  update();
}

void
ScatterWidget::mouseReleaseEvent(QMouseEvent *event)
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
ScatterWidget::px2loc(QPointF const &px) const
{
  auto result = (px - .5 * QPointF(m_viewRect.width(), m_viewRect.height()) - m_gridTopLeft - m_currPos) * ds();
  result.setY(-result.y());

  return result;
}

QPointF
ScatterWidget::loc2px(QPointF const &input) const
{
  QPointF loc = input;

  loc.setY(-loc.y());

  return loc / ds() + .5 * QPointF(m_viewRect.width(), m_viewRect.height()) + m_currPos + m_gridTopLeft;
}

void
ScatterWidget::resizeEvent(QResizeEvent *)
{
  qreal oldWidth = m_viewRect.width();

  m_gridTopLeft = QPointF(m_leftMargin, m_topMargin);
  m_gridBottomRight = QPointF(
        width() - m_rightMargin - 1,
        height() - m_bottomMargin - 1);

  m_viewRect = QRectF(m_gridTopLeft, m_gridBottomRight);


  qreal currWidth = m_viewRect.width();

  if (m_firstResize) {
    oldWidth = m_viewRect.width();
    m_firstResize = false;
    triggerNewView();
  }

  m_zoom       *= oldWidth / currWidth;
  m_renderZoom *= oldWidth / currWidth;

  auto ds   = this->ds();
  m_currPos = -QPointF(m_x0 / ds, -m_y0 / ds);

  update();
  requestRender();
}

void
ScatterWidget::mouseMoveEvent(QMouseEvent *event)
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
ScatterWidget::wheelEvent(QWheelEvent *event)
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
ScatterWidget::requestRender()
{
  if (m_numPoints > SCATTER_WIDGET_ASYNC_THRESHOLD) {
    ++m_reqId;
    m_asyncRenderer->setLastReqId(m_reqId);
    emit render(m_reqId, m_zoom, m_x0, m_y0, m_viewRect.width(), m_viewRect.height());
  } else {
    m_needsNewView = true;
    update();
  }
}

void
ScatterWidget::setCurrentRenderAsReference()
{
  m_renderPos    = m_currPos;
  m_renderZoom   = m_zoom;
  m_renderDs     = ds();
  m_needsNewView = false;
}

void
ScatterWidget::onViewReady()
{
  requestRender();
}

void
ScatterWidget::onComplete(qint64 reqId, QImage *image)
{
  if (m_reqId == reqId) {
    m_image = *image;
    m_asyncRenderer->returnImage(image);

    setCurrentRenderAsReference();
    update();
  }
}
