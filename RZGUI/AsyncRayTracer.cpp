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

#include "AsyncRayTracer.h"
#include <OMModel.h>
#include <QMutexLocker>

AsyncRayTracer::AsyncRayTracer(RZ::OMModel *model, QObject *parent)
  : QObject{parent}
{
  m_model = model;
  m_cancelled = false;

  setNonSeq(false);
  setCalculateFields(false);

  m_tracingProperties.listener        = this;
  m_tracingProperties.clearDetectors  = false;
  m_tracingProperties.maxPropagations = 8;
}

bool
AsyncRayTracer::setModel(RZ::OMModel *model)
{
  if (m_running)
    return false;

  m_model = model;
  return true;
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
AsyncRayTracer::setNonSeq(bool nonSeq)
{
  m_tracingProperties.type = nonSeq ? RZ::NonSequential : RZ::Sequential;
}

void
AsyncRayTracer::setBeam(RZ::RayList const &beam)
{
  QMutexLocker<QMutex> locker(&m_beamMutex);
  m_tracingProperties.pRays = &beam;
}

void
AsyncRayTracer::setCalculateFields(bool doIt)
{
  m_tracingProperties.calculateFields = doIt;
}

void
AsyncRayTracer::setSecondaryRays(bool doIt)
{
  m_tracingProperties.secondaryRays = doIt;
}

void
AsyncRayTracer::setMaxPropagations(unsigned int maxProp)
{
  m_tracingProperties.maxPropagations = maxProp;
}

void
AsyncRayTracer::setCompactifyInterval(unsigned int interval)
{
  m_tracingProperties.compactifyInterval = interval;
}

void
AsyncRayTracer::setKeepStrayLight(bool doIt)
{
  m_tracingProperties.keepStrayRays = doIt;
}

void
AsyncRayTracer::setAccumulate(bool acc)
{
  m_tracingProperties.clearPrevious = !acc;
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

  m_tracingProperties.beamElement = m_updateBeam ? m_model->beam() : nullptr;
  m_tracingProperties.startTime   = &m_batchStart;
  
  
  if (m_tracingProperties.pRays == nullptr) {
    emit error("Undefined beam object");
  } else {
    try {
      m_running = true;
      
      m_model->simulation()->trace(m_tracingProperties);

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
