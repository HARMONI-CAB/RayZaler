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

#include "AsyncDataProductRenderer.h"
#include <DataProduct.h>
#include <sys/time.h>

AsyncDataProductRenderer::AsyncDataProductRenderer(
    RZ::DataProduct *product,
    QObject *parent) :
  QObject(parent)
{
  m_product = product;
}

AsyncDataProductRenderer::~AsyncDataProductRenderer()
{
  for (auto p : m_allImages)
    delete p;
}

void
AsyncDataProductRenderer::clearImagePool()
{
  for (auto p : m_pool) {
    m_allImages.erase(p);
    delete p;
  }

  m_pool.clear();
}

void
AsyncDataProductRenderer::setLastReqId(qint64 reqId)
{
  QMutexLocker<QMutex> locker(&m_mutex);

  m_lastReqId = reqId;
}

QImage *
AsyncDataProductRenderer::allocImage(QSize const &size)
{
  QImage *newImage = nullptr;
  QMutexLocker<QMutex> locker(&m_poolMutex);

  if (m_poolImgSize != size) {
    clearImagePool();
    m_poolImgSize = size;
  }

  if (m_pool.empty()) {
    newImage = new QImage(size.width(), size.height(), QImage::Format_ARGB32);
    newImage->fill(0);
    m_allImages.insert(newImage);
  } else {
    newImage = m_pool.front();
    m_pool.pop_front();
  }

  return newImage;
}

void
AsyncDataProductRenderer::discardCurrentView()
{
  m_haveView = false;
}

bool
AsyncDataProductRenderer::haveView() const
{
  return m_haveView;
}

void
AsyncDataProductRenderer::returnImage(QImage *image)
{
  QMutexLocker<QMutex> locker(&m_poolMutex);

  if (image->size() != m_poolImgSize) {
    m_allImages.erase(image);
    delete image;
  } else {
    m_pool.push_back(image);
    image->fill(0);
  }
}

void
AsyncDataProductRenderer::makeView()
{
  struct timeval tv, otv, diff;

  gettimeofday(&otv, nullptr);
  m_product->prepareView();
  gettimeofday(&tv, nullptr);
  timersub(&tv, &otv, &diff);

  m_haveView = true;

  emit viewReady();
}

void
AsyncDataProductRenderer::clearData()
{
  m_product->clear();
  emit viewReady();
}

void
AsyncDataProductRenderer::saveData(QString data)
{
  if (!m_product->saveToFile(data.toStdString()))
    emit error("Failed to save data to file: " + QString(strerror(errno)));
}

void
AsyncDataProductRenderer::render(
    qint64 reqId,
    qreal zoom,
    qreal x0,
    qreal y0,
    int width,
    int height)
{
  bool doRender = false;

  m_mutex.lock();
  doRender = m_lastReqId <= reqId;
  if (doRender)
    m_lastReqId = reqId;
  m_mutex.unlock();

  if (doRender) {
    QImage *image = allocImage(QSize(width, height));

    if (m_haveView) {
      renderToImage(*image, zoom, x0, y0);
    } else {
      image->fill(0xffbfbfbf);
    }

    emit complete(reqId, image);
  }
}
