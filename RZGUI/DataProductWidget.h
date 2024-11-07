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

#ifndef DATAPRODUCTWIDGET_H
#define DATAPRODUCTWIDGET_H

#include <QObject>
#include <QWidget>
#include <DataProduct.h>
#include <QThread>
#include <QMap>
#include "AsyncDataProductRenderer.h"

struct RenderInfo {
  QPointF pos;
  qreal   zoom = 1.;
  qreal   ds   = 1.;
};

class DataProductWidget : public QWidget
{
  Q_OBJECT

  RZ::DataProduct *m_product = nullptr;

  // Render contents
  QImage       m_image;
  qreal        m_x0   = 0;
  qreal        m_y0   = 0;
  qreal        m_zoom = 1;
  bool         m_firstResize = true;

  // View location
  int          m_topMargin = 0;
  int          m_leftMargin = 0;
  int          m_bottomMargin = 0;
  int          m_rightMargin = 0;
  QPointF      m_gridTopLeft;
  QPointF      m_gridBottomRight;
  QRectF       m_viewRect;

  // Representation properties
  QFont        m_tickFont;
  QFontMetrics m_fontMetrics;
  bool         m_interactive = true;

  // Grid state
  qreal        m_bestFineStep;
  qreal        m_bestCoarseStep;
  qreal        m_bestUnitDivider = 1.;
  QString      m_bestUnits = "m";

  // Display state
  bool         m_movingSelection = false;
  bool         m_haveRefPos  = false;
  bool         m_haveLastPos = false;

  bool         m_needsNewView = true;
  RenderInfo   m_lastRender;
  QPointF      m_moveRefPos;
  QPointF      m_moveLastPos;
  QPointF      m_currPos;

  QMap<qint64, RenderInfo>  m_renderHistory;

  // Rendering objects
  QThread                   m_renderThread;
  AsyncDataProductRenderer *m_asyncRenderer = nullptr;
  qint64                    m_reqId = 0;

  QPointF      px2loc(QPointF const &) const;
  QPointF      loc2px(QPointF const &) const;
  void         paintGrid(QPainter &) const;
  void         paintTicks(QPainter &) const;
  void         guessScale();
  qreal        ds() const;
  void         requestRender();
  void         setCurrentRenderAsReference();

  void         paintLastRender(QPainter &painter);
  void         paintBusyMessage(QPainter &painter);
  void         paintLabels(QPainter &painter);

protected:
  AsyncDataProductRenderer *asyncRenderer();
  virtual AsyncDataProductRenderer *makeRenderer(RZ::DataProduct *) = 0;

public:
  DataProductWidget(RZ::DataProduct *, QWidget *);
  virtual ~DataProductWidget() override;

  // Reimplemented events
  void    resizeEvent(QResizeEvent *) override;
  void    paintEvent(QPaintEvent *) override;
  void    mousePressEvent(QMouseEvent *) override;
  void    mouseReleaseEvent(QMouseEvent *) override;
  void    mouseMoveEvent(QMouseEvent *) override;
  void    wheelEvent(QWheelEvent *) override;

  void    resetZoom();
  void    updateView();

public slots:
  void    onComplete(qint64, QImage *);
  void    onViewReady();

signals:
  void    viewChanged();
  void    makeView();
  void    render(qint64, qreal, qreal, qreal, int, int);
};

#endif // DATAPRODUCTWIDGET_H
