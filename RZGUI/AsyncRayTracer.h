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
