#include "ScatterAsyncRenderer.h"
#include "ScatterTree.h"
#include "ScatterPainter.h"
#include <sys/time.h>

ScatterAsyncRenderer::ScatterAsyncRenderer(QObject *parent, ScatterTree *tree) :
  QObject(parent)
{
  m_tree = tree;
}

ScatterAsyncRenderer::~ScatterAsyncRenderer()
{
  for (auto p : m_allImages)
    delete p;
}

void
ScatterAsyncRenderer::clearImagePool()
{
  for (auto p : m_pool) {
    m_allImages.erase(p);
    delete p;
  }

  m_pool.clear();
}

void
ScatterAsyncRenderer::setLastReqId(qint64 reqId)
{
  QMutexLocker<QMutex> locker(&m_mutex);

  m_lastReqId = reqId;
}

QImage *
ScatterAsyncRenderer::allocImage(QSize const &size)
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
ScatterAsyncRenderer::discardCurrentView()
{
  m_haveView = false;
}

bool
ScatterAsyncRenderer::haveView() const
{
  return m_haveView;
}

void
ScatterAsyncRenderer::returnImage(QImage *image)
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
ScatterAsyncRenderer::makeView()
{
  struct timeval tv, otv, diff;

  printf("Rebuilding...\n");
  gettimeofday(&otv, nullptr);
  m_tree->rebuild();
  gettimeofday(&tv, nullptr);
  timersub(&tv, &otv, &diff);
  printf("Done. %ld.%06ld s\n", diff.tv_sec, diff.tv_usec);

  m_tree->setFinestScale(5);
  m_haveView = true;

  emit viewReady();
}

void
ScatterAsyncRenderer::render(
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
      ScatterPainter renderer(*image, zoom, x0, y0);
      m_tree->render(&renderer);
    } else {
      image->fill(0xffbfbfbf);
    }

    emit complete(reqId, image);
  }
}
