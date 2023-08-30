#ifndef SIMULATIONPROGRESSDIALOG_H
#define SIMULATIONPROGRESSDIALOG_H

#include <QDialog>
#include <sys/time.h>

namespace Ui {
  class SimulationProgressDialog;
}

class AsyncRayTracer;

class SimulationProgressDialog : public QDialog
{
  Q_OBJECT

  AsyncRayTracer *m_tracer = nullptr;
  bool m_cancelled = false;
  struct timeval m_openTime;
  bool m_opened = false;
  void connectAll();
  unsigned int m_count = 0;
  unsigned int m_maxSim = 1;
public:
  explicit SimulationProgressDialog(AsyncRayTracer *, QWidget *parent = nullptr);
  ~SimulationProgressDialog();

  void simFinished();
  virtual void reject() override;
  virtual void open() override;

  void setPath(QString);
  void setMaxSim(unsigned int);

public slots:
  void onProgress(int, int);
  void onGlobalProgress(QString, int, int);
  void onFinished();
  void onAborted();
  void onError(QString);

private:
  Ui::SimulationProgressDialog *ui;
};

#endif // SIMULATIONPROGRESSDIALOG_H
