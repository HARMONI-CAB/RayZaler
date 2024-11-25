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

#ifndef ASYNCRAYTRACER_H
#define ASYNCRAYTRACER_H

#include <QObject>
#include <RayTracingEngine.h>
#include <QMutex>

namespace RZ {
  class OMModel;
};

class AsyncRayTracer : public QObject, public RZ::RayTracingProcessListener
{
  Q_OBJECT

  RZ::OMModel              *m_model       = nullptr; // Borrowed
  const std::list<RZ::Ray> *m_beam        = nullptr;
  QMutex                    m_beamMutex;
  bool                      m_cancelled   = false;
  bool                      m_diffraction = false;
  bool                      m_running     = false;
  bool                      m_updateBeam  = true;
  bool                      m_accumulate  = false;
  int                       m_currSim     = 0;
  int                       m_numSim      = 1;
  struct timeval            m_batchStart;

public:
  explicit AsyncRayTracer(RZ::OMModel *model, QObject *parent = nullptr);

  bool setModel(RZ::OMModel *model);
  void cancel();
  void setUpdateBeam(bool);
  void setDiffraction(bool);
  void setBeam(std::list<RZ::Ray> const &);
  void setAccumulate(bool);
  bool running() const;

  // Overriden methods
  virtual void stageProgress(
          RZ::RayTracingStageProgressType,
          std::string const &,
          unsigned int num,
          unsigned int total) override;

  virtual void rayProgress(uint64_t num, uint64_t total) override;

  virtual uint64_t rayNotifyInterval() const override;
  virtual bool cancelled() const override;


signals:
  void progress(int, int);
  void globalProgress(QString, int, int);
  void aborted();
  void finished(bool);
  void error(QString);

public slots:
  void onStartRequested(QString, int, int); // Actually calls trace()

};

#endif // ASYNCRAYTRACER_H
