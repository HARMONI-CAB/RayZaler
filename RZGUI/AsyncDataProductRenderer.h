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
  void clearData();
  void saveData(QString);

signals:
  void complete(qint64, QImage *);
  void viewReady();
  void error(QString);
};

#endif // ASYNCDATAPRODUCTRENDERER_H
