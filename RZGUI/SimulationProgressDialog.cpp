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

#include "SimulationProgressDialog.h"
#include "ui_SimulationProgressDialog.h"
#include "AsyncRayTracer.h"
#include <QMessageBox>

SimulationProgressDialog::SimulationProgressDialog(AsyncRayTracer *tracer, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SimulationProgressDialog)
{
  ui->setupUi(this);

  setTracer(tracer);
}

void
SimulationProgressDialog::setTracer(AsyncRayTracer *tracer)
{
  if (m_tracer != nullptr) {
    disconnect(this, SLOT(onProgress(int, int)));
    disconnect(this, SLOT(onGlobalProgress(QString, int, int)));
    disconnect(this, SLOT(onFinished()));
    disconnect(this, SLOT(onAborted()));
    disconnect(this, SLOT(onError(QString)));
  }

  m_tracer = tracer;

  if (m_tracer != nullptr)
    connectAll();
}


void
SimulationProgressDialog::connectAll()
{
  connect(
        m_tracer,
        SIGNAL(progress(int, int)),
        this,
        SLOT(onProgress(int, int)));

  connect(
        m_tracer,
        SIGNAL(globalProgress(QString, int, int)),
        this,
        SLOT(onGlobalProgress(QString, int, int)));

  connect(
        m_tracer,
        SIGNAL(finished(bool)),
        this,
        SLOT(onFinished()));

  connect(
        m_tracer,
        SIGNAL(aborted()),
        this,
        SLOT(onAborted()));

  connect(
        m_tracer,
        SIGNAL(error(QString)),
        this,
        SLOT(onError(QString)));
}

SimulationProgressDialog::~SimulationProgressDialog()
{
  delete ui;
}

void
SimulationProgressDialog::setPath(QString path)
{
  if (path.size() == 0)
    ui->stateLabel->setText("Running simulation on default path");
  else
    ui->stateLabel->setText("Running simulation on path" + path);
}

void
SimulationProgressDialog::setMaxSim(unsigned int maxSim)
{
  m_maxSim = maxSim;
}

void
SimulationProgressDialog::simFinished()
{
  m_opened = false;
  accept();
}

void
SimulationProgressDialog::reject()
{
  m_tracer->cancel();
}

void
SimulationProgressDialog::open()
{
  struct timeval diff = {0, 300000};
  struct timeval tv;

  m_cancelled = false;
  ui->stepProgressBar->setValue(0);
  ui->stepProgressBar->setFormat("Starting...");
  ui->simProgressBar->setFormat("Starting...");

  if (!m_opened) {
    ui->simProgressBar->setValue(0);
    gettimeofday(&tv, nullptr);
    timeradd(&tv, &diff, &m_openTime);
    m_opened = true;
  }
}

void
SimulationProgressDialog::onProgress(int n, int total)
{
  if (m_opened && !isVisible()) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    if (timercmp(&tv, &m_openTime, >))
      QDialog::open();
  }

  ui->stepProgressBar->setFormat(QString::number(n) + "/" + QString::number(total));
  ui->stepProgressBar->setMinimum(0);
  ui->stepProgressBar->setValue(n);
  ui->stepProgressBar->setMaximum(total);
}

void
SimulationProgressDialog::onGlobalProgress(QString desc, int n, int total)
{
  if (m_opened && !isVisible()) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    if (timercmp(&tv, &m_openTime, >))
      QDialog::open();
  }

  if (m_cancelled) {
    ui->simProgressBar->setFormat("Cancelling...");
  } else {
    ui->simProgressBar->setFormat(desc);
    ui->simProgressBar->setMinimum(0);
    ui->simProgressBar->setValue(n);
    ui->simProgressBar->setMaximum(total);
    ui->simProgressBar->setFormat(
          desc + 
          " ("
          + QString::number(n)
          + "/"
          + QString::number(total)
          + ")");
  }
}

void
SimulationProgressDialog::onFinished()
{

}

void
SimulationProgressDialog::onAborted()
{
  m_opened = false;
  QDialog::reject();
}

void
SimulationProgressDialog::onError(QString err)
{
  QMessageBox::critical(
        this,
        "Ray tracer engine error",
        "Ray tracer engine error: " + err);
  QDialog::reject();
}
