#ifndef ASYNCDATAPRODUCTRENDERER_H
#define ASYNCDATAPRODUCTRENDERER_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <list>
#include <set>

namespace RZ {
  class DataProduct;
}

class AsyncDataProductRenderer : public QObject
{
  Q_OBJECT

  RZ::DataProduct    *m_product; // Borrowed
  bool                m_haveView = false;
  QMutex              m_mutex;
  qint64              m_lastReqId = 0;

  QMutex              m_poolMutex;
  std::set<QImage *>  m_allImages;
  std::list<QImage *> m_pool;
  QSize               m_poolImgSize;

  void                clearImagePool();
  QImage             *allocImage(QSize const &);

public:
  AsyncDataProductRenderer(RZ::DataProduct *, QObject *parent);
  virtual ~AsyncDataProductRenderer() override;

  virtual void renderToImage(QImage &, qreal, qreal, qreal) = 0;
  virtual bool isBig() const = 0;

  void setLastReqId(qint64 reqId);
  void discardCurrentView();
  void returnImage(QImage *);
  bool haveView() const;

public slots:
  void render(qint64, qreal, qreal, qreal, int, int);
  void makeView();

signals:
  void complete(qint64, QImage *);
  void viewReady();
};

#endif // ASYNCDATAPRODUCTRENDERER_H
