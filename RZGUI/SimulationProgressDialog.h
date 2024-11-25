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
  void setTracer(AsyncRayTracer *);

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
