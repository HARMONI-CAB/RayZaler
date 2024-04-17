#include "SessionTabWidget.h"
#include "SimulationSession.h"
#include "ui_SessionTabWidget.h"
#include <QMessageBox>

#include "RZGUIGLWidget.h"
#include "SimulationProgressDialog.h"
#include "DetectorWindow.h"
#include <RayBeamElement.h>
#include "SourceEditorWindow.h"

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


  FILE *fp = fopen(session->path().toStdString().c_str(), "rb");

  if (fp != nullptr) {
    m_sourceEditorWindow = new SourceEditorWindow(this);
    m_sourceEditorWindow->loadFromFp(fp);
    m_sourceEditorWindow->setFileName(session->fileName().toStdString());
    fclose(fp);
  }

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

  connect(
        m_sourceEditorWindow,
        SIGNAL(build()),
        this,
        SLOT(onSourceEditorBuild()));
}

SessionTabWidget::~SessionTabWidget()
{
  delete m_progressDialog;
  delete m_sourceEditorWindow;
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
SessionTabWidget::showSourceWindow()
{
  m_sourceEditorWindow->show();
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

void
SessionTabWidget::reloadModelFromEditor()
{
  m_detWindow->setSession(nullptr);
  m_glWidget->setModel(nullptr);
  m_progressDialog->setTracer(nullptr);

  try {
    auto *recipe = new RZ::Recipe();
    recipe->addDof("t", 0, 0, 1e6);
    auto *ctx = m_sourceEditorWindow->makeParserContext(recipe);

    ctx->setFile(m_session->fileName().toStdString());
    ctx->addSearchPath(m_session->searchPath().toStdString());

    // Will release memory always, even after an error
    m_session->reload(ctx);
  } catch (RZ::ParserError const &e) {
    m_sourceEditorWindow->highlightError(
          e.file(),
          e.line(),
          e.col(),
          e.message());
  } catch (std::runtime_error const &e) {
    QMessageBox::critical(
          m_sourceEditorWindow,
          "Build error",
          "Model has errors: " + QString(e.what()));
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

void
SessionTabWidget::onSourceEditorBuild()
{
  reloadModelFromEditor();
}
