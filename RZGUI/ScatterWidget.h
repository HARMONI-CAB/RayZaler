#ifndef SCATTERWIDGET_H
#define SCATTERWIDGET_H

#include <QWidget>
#include <QThread>
#include <QFont>
#include <QFontMetrics>

class ScatterTree;
class ScatterAsyncRenderer;

#define SCATTER_WIDGET_ASYNC_THRESHOLD 50000

class ScatterWidget : public QWidget
{
  Q_OBJECT

  ScatterTree *m_tree = nullptr;

  unsigned int m_numPoints = 2 * SCATTER_WIDGET_ASYNC_THRESHOLD - 1;
  QImage       m_image;
  qreal        m_x0   = 0;
  qreal        m_y0   = 0;
  qreal        m_zoom = 1;
  bool         m_firstResize = true;

  int          m_topMargin = 0;
  int          m_leftMargin = 0;
  int          m_bottomMargin = 0;
  int          m_rightMargin = 0;
  QPointF      m_gridTopLeft;
  QPointF      m_gridBottomRight;
  QRectF       m_viewRect;

  QFont        m_tickFont;
  QFontMetrics m_fontMetrics;

  bool         m_needsNewView = true;
  qreal        m_bestFineStep;
  qreal        m_bestCoarseStep;
  qreal        m_bestUnitDivider = 1.;
  QString      m_bestUnits = "m";

  bool         m_interactive = true;
  bool         m_movingSelection = false;
  bool         m_haveRefPos  = false;
  bool         m_haveLastPos = false;

  QPointF      m_renderPos;
  QPointF      m_renderOrigin;
  QPointF      m_renderCorner;
  qreal        m_renderZoom = 1.;
  qreal        m_renderDs = 1;
  QPointF      m_moveRefPos;
  QPointF      m_moveLastPos;
  QPointF      m_currPos;

  QThread      m_renderThread;
  ScatterAsyncRenderer *m_asyncRenderer = nullptr;
  qint64       m_reqId = 0;

  QPointF      px2loc(QPointF const &) const;
  QPointF      loc2px(QPointF const &) const;
  void         paintGrid(QPainter &) const;
  void         paintTicks(QPainter &) const;
  void         guessScale();
  qreal        ds() const;
  void         requestRender();
  void         setCurrentRenderAsReference();
  void         triggerNewView();

public:
  explicit ScatterWidget(QWidget *parent = nullptr);
  virtual ~ScatterWidget() override;

  void    resetZoom();

  void    resizeEvent(QResizeEvent *) override;
  void    paintEvent(QPaintEvent *) override;
  void    mousePressEvent(QMouseEvent *) override;
  void    mouseReleaseEvent(QMouseEvent *) override;
  void    mouseMoveEvent(QMouseEvent *) override;
  void    wheelEvent(QWheelEvent *) override;

public slots:
  void    onComplete(qint64, QImage *);
  void    onViewReady();

signals:
  void    viewChanged();
  void    makeView();
  void    render(qint64, qreal, qreal, qreal, int, int);
};

#endif // SCATTERWIDGET_H
