#ifndef IMAGENAVWIDGET_H
#define IMAGENAVWIDGET_H

#include <QWidget>
#include <QPaintEvent>

namespace RZ {
  class Detector;
}

class QImage;
class QPointF;

class ImageNavWidget : public QWidget
{
  Q_OBJECT

  RZ::Detector *m_detector      = nullptr;
  QImage        m_image;
  std::vector<uint8_t> m_asBytes;
  qreal         m_zoom          = 1.0;
  qreal         m_ratio         = 1.;
  qreal         m_preferredZoom = 1.0;
  qreal         m_arr_min       = 0;
  qreal         m_arr_max       = 65536;

  qreal         m_sane_min      = 0;
  qreal         m_sane_max      = 1;

  int           m_img_width     = 0;
  int           m_img_height    = 0;

  QPointF       m_move_last_pos;
  QPointF       m_move_ref_pos;
  QPointF       m_currPos;
  QPoint        m_currSel;

  bool          m_have_last_pos = false;
  bool          m_have_ref_pos  = false;

  bool          m_interactive   = true;
  bool          m_autoscale     = false;

  bool          m_movingSelection = false;
  bool          m_logScale = false;
  bool          m_showPhotons = true;
  bool          m_showPhase = false;
  bool          m_showGrid = false;

  QPoint px2img(QPoint) const;
  QPoint img2px(QPoint) const;

  QPointF px2img(QPointF) const;
  QPointF img2px(QPointF) const;

  QPoint px2imgcenter(QPoint) const;
  QPointF px2imgcenter(QPointF) const;

  QPoint imgcenter2px(QPoint) const;
  QPointF imgcenter2px(QPointF) const;

  static QPoint boundToRect(QPoint const &, QRect const &);
  static QPointF boundToRect(QPointF const &, QRectF const &);


  void paintGrid(QPainter &) const;

  inline void psetRGB(unsigned p, qreal norm, qreal phase);
  inline qreal pixelValue(unsigned p);
  inline qreal pixelPhase(unsigned p);

  static inline uint8_t
  pixBound(qreal quantity)
  {
    return static_cast<uint8_t>(
          qBound(
            static_cast<int32_t>(0),
            static_cast<int32_t>(quantity),
            static_cast<int32_t>(255)));
  }

public:
  explicit ImageNavWidget(QWidget *parent = nullptr);

  QPointF getSelection() const;
  void    recalcImage();
  void    setInteractive(bool);
  void    setImageLimits(qreal min, qreal max);
  void    setAutoScale(bool);
  void    setDetector(RZ::Detector *);
  void    setZoom(qreal);
  void    resetZoom();
  void    zoomToPoint(QPointF const &);
  
  void    setShowPhotons(bool);
  void    setShowPhase(bool);
  void    setShowGrid(bool);

  qreal   imgMin() const;
  qreal   imgMax() const;

  void    setSelection(QPoint const &);
  void    updateSelectionFromEvent(QMouseEvent *, bool last = false);
  void    setLogScale(bool);
  QSize   imageSize() const;
  QSizeF  viewSize() const;
  QPointF currPoint() const;
  qreal   zoom() const;

  void    setCurrPoint(QPointF const &);
  void    paintEvent(QPaintEvent *) override;
  void    mousePressEvent(QMouseEvent *) override;
  void    mouseReleaseEvent(QMouseEvent *) override;
  void    mouseMoveEvent(QMouseEvent *) override;
  void    wheelEvent(QWheelEvent *) override;

signals:
  void selChanged(bool);
  void mouseMoved(QPointF);
  void viewChanged();
};

#endif // IMAGENAVWIDGET_H
