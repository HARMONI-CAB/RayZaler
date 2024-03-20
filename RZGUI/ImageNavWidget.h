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

  QPoint px2img(QPoint) const;
  QPoint img2px(QPoint) const;
  QPoint px2imgcenter(QPoint) const;
  QPointF pxF2imgcenterF(QPointF) const;
  QPoint imgcenter2px(QPoint) const;

  inline qreal pixelValue(unsigned p);

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
