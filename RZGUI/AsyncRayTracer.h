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

  RZ::OMModel              *m_model     = nullptr; // Borrowed
  const std::list<RZ::Ray> *m_beam      = nullptr;
  QMutex                    m_beamMutex;
  bool                      m_cancelled = false;
  bool                      m_running   = false;

public:
  explicit AsyncRayTracer(RZ::OMModel *model, QObject *parent = nullptr);

  void cancel();
  void setBeam(std::list<RZ::Ray> const &);
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
  void finished();
  void error(QString);

public slots:
  void onStartRequested(QString); // Actually calls trace()

};

#endif // ASYNCRAYTRACER_H