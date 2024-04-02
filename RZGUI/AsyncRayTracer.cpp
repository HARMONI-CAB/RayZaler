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
AsyncRayTracer::setUpdateBeam(bool update)
{
  m_updateBeam = update;
}

void
AsyncRayTracer::setDiffraction(bool diffraction)
{
  m_diffraction = diffraction;
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

    case RZ::PROGRESS_TYPE_KIRCHHOFF:
      progressString = "Integrating wavefront in " + strObj;
      break;

    case RZ::PROGRESS_TYPE_TRANSFER:
      progressString = "Transferring rays from " + strObj;
      break;
  }

  if (m_numSim > 1)
    emit globalProgress(progressString, m_currSim, m_numSim);
  else
    emit globalProgress(progressString, total * m_currSim + num, total * m_numSim);
}

void
AsyncRayTracer::rayProgress(uint64_t num, uint64_t total)
{
  emit progress(num / 3, total / 3);
}

uint64_t
AsyncRayTracer::rayNotifyInterval() const
{
  return 250;
}

bool
AsyncRayTracer::cancelled() const
{
  return m_cancelled;
}

void
AsyncRayTracer::onStartRequested(QString path, int step, int total)
{
  QMutexLocker<QMutex> locker(&m_beamMutex);

  if (step == 0) {
    m_cancelled = false;
    gettimeofday(&m_batchStart, nullptr);
  }

  m_currSim = step;
  m_numSim  = total;

  if (m_beam == nullptr) {
    emit error("Undefined beam object");
  } else {
    try {
      m_running = true;

      if (m_diffraction) {
        m_model->traceDiffraction(
              path.toStdString(),
              *m_beam,
              this,
              false,
              &m_batchStart);
      } else {
        m_model->trace(
              path.toStdString(),
              *m_beam,
              m_updateBeam,
              this,
              false,
              &m_batchStart);
      }

      m_batchStart = m_model->lastTracerTick();
      m_running = false;
      if (m_cancelled)
        emit aborted();
      else
        emit finished(m_updateBeam);
    } catch (std::runtime_error const &e) {
      m_running = false;
      emit error("Tracer error: " + QString::fromStdString(e.what()));
    }
  }
}
