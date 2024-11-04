#ifndef SCATTERASYNCRENDERER_H
#define SCATTERASYNCRENDERER_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <list>
#include <set>

class ScatterTree;

class ScatterAsyncRenderer : public QObject
{
  Q_OBJECT

  ScatterTree *m_tree; // Borrowed
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
  ScatterAsyncRenderer(QObject *parent, ScatterTree *);
  virtual ~ScatterAsyncRenderer() override;
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

#endif // SCATTERASYNCRENDERER_H
