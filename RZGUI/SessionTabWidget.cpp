#include "SessionTabWidget.h"
#include "SimulationSession.h"
#include "ui_SessionTabWidget.h"
#include <QMessageBox>

#include "RZGUIGLWidget.h"
#include "SimulationProgressDialog.h"
#include "DetectorWindow.h"
#include <RayBeamElement.h>

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

  m_detWindow = new DetectorWindow(this);
  m_detWindow->setSession(m_session);

  connectAll();
}

bool
SessionTabWidget::displayNames() const
{
  return m_displayNames;
}

void
SessionTabWidget::setDisplayNames(bool disp)
{
  m_displayNames = disp;
  m_glWidget->setDisplayNames(disp);
}

bool
SessionTabWidget::displayElements() const
{
  return m_displayElements;
}

void
SessionTabWidget::setDisplayElements(bool disp)
{
  m_displayElements = disp;
  m_glWidget->setDisplayElements(disp);
}

bool
SessionTabWidget::displayApertures() const
{
  return m_displayApertures;
}

void
SessionTabWidget::setDisplayApertures(bool disp)
{
  m_displayApertures = disp;
  m_glWidget->setDisplayApertures(disp);
}

void
SessionTabWidget::setSelectedReferenceFrame(RZ::ReferenceFrame *frame)
{
  m_glWidget->setSelectedReferenceFrame(frame);
}

void
SessionTabWidget::setSelectedOpticalPath(const RZ::OpticalPath *path)
{
  m_glWidget->setSelectedOpticalPath(path);
}

bool
SessionTabWidget::displayRefFrames() const
{
  return m_displayRefFrames;
}

void
SessionTabWidget::setDisplayRefFrames(bool disp)
{
  m_displayRefFrames = disp;
  m_glWidget->setDisplayRefFrames(disp);
}

void
SessionTabWidget::keyPressEvent(QKeyEvent *event)
{
  m_glWidget->keyPressEvent(event);

  QWidget::keyPressEvent(event);
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
SessionTabWidget::clearBeam()
{
  RZ::RayBeamElement *beam = static_cast<RZ::RayBeamElement *>(m_session->topLevelModel()->beam());

  beam->clear();
  m_glWidget->update();
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
SessionTabWidget::reloadModel()
{
  m_detWindow->setSession(nullptr);
  m_glWidget->setModel(nullptr);
  m_progressDialog->setTracer(nullptr);

  try {
    m_session->reload();
  } catch (std::runtime_error const &e) {
    QMessageBox::critical(
          this,
          "Reload model",
          "Cannot reload model: " + QString::fromStdString(e.what()));
  }

  m_glWidget->setModel(m_session->topLevelModel());
  m_detWindow->setSession(m_session);
  m_progressDialog->setTracer(m_session->tracer());
}

////////////////////////////////// Slots ///////////////////////////////////////
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

