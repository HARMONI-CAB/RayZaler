#include "SessionTabWidget.h"
#include "SimulationSession.h"
#include "ui_SessionTabWidget.h"

#include "RZGUIGLWidget.h"
#include "SimulationProgressDialog.h"
#include "DetectorWindow.h"

SessionTabWidget::SessionTabWidget(
    SimulationSession *session,
    QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SessionTabWidget)
{
  m_session = session;

  ui->setupUi(this);

  m_glWidget = new RZGUIGLWidget(this);
  ui->mainGrid->addWidget(m_glWidget, 0, 0, 1, 1);

  m_glWidget->setModel(session->topLevelModel());

  m_progressDialog = new SimulationProgressDialog(session->tracer(), this);
  m_progressDialog->setWindowTitle(
        "Simulation progress - " + m_session->fileName());

  m_detWindow = new DetectorWindow();
  m_detWindow->setSession(m_session);

  connectAll();
}

void
SessionTabWidget::connectAll()
{
  connect(
        m_session,
        SIGNAL(modelChanged()),
        this,
        SLOT(onModelChanged()));

  connect(
        m_session,
        SIGNAL(triggerSimulation(QString, int, int)),
        this,
        SLOT(onSimulationTriggered(QString, int, int)));

  connect(
        m_session,
        SIGNAL(sweepFinished()),
        this,
        SLOT(onSweepFinished()));
}

SessionTabWidget::~SessionTabWidget()
{
  delete m_progressDialog;
  delete ui;
}

SimulationSession *
SessionTabWidget::session() const
{
  return m_session;
}

void
SessionTabWidget::updateModel()
{
  m_glWidget->update();
}

void
SessionTabWidget::setRotation(qreal x, qreal y, qreal z)
{
  GLfloat rot[3] = {
    static_cast<GLfloat>(x),
    static_cast<GLfloat>(y),
    static_cast<GLfloat>(z)};

  m_glWidget->setCurrentRot(rot);
}

void
SessionTabWidget::showDetectorWindow()
{
  m_detWindow->show();
}

void
SessionTabWidget::updateDetectorWindow()
{
  m_detWindow->refreshImage();
}

void
SessionTabWidget::onModelChanged()
{
  updateDetectorWindow();
  updateModel();
}

void
SessionTabWidget::onSimulationTriggered(QString path, int step, int total)
{
  if (step == 0) {
    m_progressDialog->setPath(path);
    m_progressDialog->open();
  }
}

void
SessionTabWidget::onSweepFinished()
{
  m_progressDialog->simFinished();
}

