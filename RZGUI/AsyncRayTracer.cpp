#include "AsyncRayTracer.h"
#include <OMModel.h>
#include <QMutexLocker>

AsyncRayTracer::AsyncRayTracer(RZ::OMModel *model, QObject *parent)
  : QObject{parent}
{
  m_model = model;
  m_cancelled = false;
}

void
AsyncRayTracer::cancel()
{
  m_cancelled = true;
}

void
AsyncRayTracer::setBeam(std::list<RZ::Ray> const &beam)
{
  QMutexLocker<QMutex> locker(&m_beamMutex);
  m_beam = &beam;
}

bool
AsyncRayTracer::running() const
{
  return m_running;
}

void
AsyncRayTracer::stageProgress(
        RZ::RayTracingStageProgressType type,
        std::string const &object,
        unsigned int num,
        unsigned int total)
{
  QString progressString;
  QString strObj = QString::fromStdString(object);

  switch (type) {
    case RZ::PROGRESS_TYPE_CONFIG:
      progressString = "Configuring model";
      break;

    case RZ::PROGRESS_TYPE_TRACE:
      progressString = "Casting rays to " + strObj;
      break;

    case RZ::PROGRESS_TYPE_TRANSFER:
      progressString = "Transferring rays from " + strObj;
      break;
  }

  emit globalProgress(progressString, num, total);
}

void
AsyncRayTracer::rayProgress(uint64_t num, uint64_t total)
{
  emit progress(num / 3, total / 3);
}

uint64_t
AsyncRayTracer::rayNotifyInterval() const
{
  return 10000;
}

bool
AsyncRayTracer::cancelled() const
{
  return m_cancelled;
}

void
AsyncRayTracer::onStartRequested(QString path)
{
  QMutexLocker<QMutex> locker(&m_beamMutex);
  m_cancelled = false;

  if (m_beam == nullptr) {
    emit error("Undefined beam object");
  } else {
    try {
      m_running = true;
      m_model->trace(path.toStdString(), *m_beam, true, this, false);
      m_running = false;
      if (m_cancelled)
        emit aborted();
      else
        emit finished();
    } catch (std::runtime_error const &e) {
      m_running = false;
      emit error("Tracer error: " + QString::fromStdString(e.what()));
    }
  }
}
