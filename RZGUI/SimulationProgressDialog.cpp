#include "SimulationProgressDialog.h"
#include "ui_SimulationProgressDialog.h"
#include "AsyncRayTracer.h"
#include <QMessageBox>

SimulationProgressDialog::SimulationProgressDialog(AsyncRayTracer *tracer, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SimulationProgressDialog)
{
  m_tracer = tracer;

  ui->setupUi(this);

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
        SIGNAL(finished()),
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
  ui->simProgressBar->setValue(0);
  ui->stepProgressBar->setFormat("Starting...");
  ui->simProgressBar->setFormat("Starting...");

  gettimeofday(&tv, nullptr);

  timeradd(&tv, &diff, &m_openTime);
  m_opened = true;
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
  if (m_cancelled) {
    ui->simProgressBar->setFormat("Cancelling...");
  } else {
    ui->simProgressBar->setFormat(desc);
    ui->simProgressBar->setMinimum(0);
    ui->simProgressBar->setValue(n);
    ui->simProgressBar->setMaximum(total);
  }
}

void
SimulationProgressDialog::onFinished()
{
  ui->simProgressBar->setFormat("Iterationf finished");
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