#include "SessionTabWidget.h"
#include "SimulationSession.h"
#include "ui_SessionTabWidget.h"
#include <QMessageBox>
#include <QAction>

#include "GUIHelpers.h"
#include "RZGUIGLWidget.h"
#include "SimulationProgressDialog.h"
#include "DetectorWindow.h"
#include <RayBeamElement.h>
#include "SourceEditorWindow.h"
#include "ColorSettings.h"
#include "SpotDiagramWindow.h"
#include "AsyncRayTracer.h"
#include "RZGUI.h"
#include <Helpers.h>

SessionTabWidget::SessionTabWidget(
    SimulationSession *session,
    QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SessionTabWidget)
{
  ColorSettings colorSettings;

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

  addGridStep("1 µm",    1e-6);
  addGridStep("10 µm",   1e-5);
  addGridStep("15 µm", 1.5e-5);
  addGridStep("100 µm",  1e-4);
  addGridStep("1 mm",    1e-3);
  addGridStep("1 cm",    1e-2);
  addGridStep("10 cm",   1e-1);
  addGridStep("1 m",     1);

  setGridStep(1e-2);

  addGridDiv(5);
  addGridDiv(10);
  addGridDiv(20);
  addGridDiv(25);
  addGridDiv(50);
  addGridDiv(100);
  addGridDiv(250);
  addGridDiv(500);
  addGridDiv(1000);

  setGridDivs(100);

  RZGUISingleton::loadColorSettings(colorSettings);
  applyColorSettings(colorSettings);

  connectAll();
}

static inline const GLfloat *
QColorToGLfloat(QColor const &color, GLfloat *storage)
{
  storage[0] = color.redF();
  storage[1] = color.greenF();
  storage[2] = color.blueF();

  return storage;
}

void
SessionTabWidget::applyColorSettings(ColorSettings const &settings)
{
  GLfloat bgAbove[3];
  GLfloat bgBelow[3];
  GLfloat storage[3];

  m_glWidget->setBackgroundGradient(
        QColorToGLfloat(settings.bgAbove, bgAbove),
        QColorToGLfloat(settings.bgBelow, bgBelow));

  m_glWidget->setPathColor(QColorToGLfloat(settings.path, storage));
  m_glWidget->setGridColor(QColorToGLfloat(settings.grid, storage));
}

void
SessionTabWidget::addGridStep(QString const &text, qreal size)
{
  BLOCKSIG(ui->gridStepCombo, addItem(text, QVariant::fromValue<qreal>(size)));
}

void
SessionTabWidget::addGridDiv(unsigned num)
{
  BLOCKSIG(
        ui->gridDivCombo,
        addItem(QString::number(num), QVariant::fromValue<unsigned>(num)));
}

void
SessionTabWidget::setGridStep(qreal step)
{
  int index = ui->gridStepCombo->findData(QVariant::fromValue(step));

  if (index == -1) {
    addGridStep("Custom (" + asScientific(step) + " m)", step);
    index = ui->gridStepCombo->count() - 1;
  }

  m_glWidget->setGridStep(step);

  BLOCKSIG(ui->gridStepCombo, setCurrentIndex(index));
}

void
SessionTabWidget::setGridDivs(unsigned num)
{
  int index = ui->gridDivCombo->findData(QVariant::fromValue(num));

  if (index == -1) {
    addGridDiv(num);
    index = ui->gridDivCombo->count() - 1;
  }

  m_glWidget->setGridDivs(num);

  BLOCKSIG(ui->gridDivCombo, setCurrentIndex(index));
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
SessionTabWidget::setDisplayGrid(bool disp)
{
  m_displayGrid = disp;
  m_glWidget->setDisplayGrid(disp);
  ui->gridWidget->setEnabled(disp);
}

bool
SessionTabWidget::displayGrid() const
{
  return m_displayGrid;
}

const RZ::ReferenceFrame *
SessionTabWidget::selectedFrame() const
{
  return m_selectedFrame;
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
SessionTabWidget::setSelectedReferenceFrame(RZ::ReferenceFrame *frame, const char *name)
{
  m_selectedFrame = frame;
  m_glWidget->setSelectedReferenceFrame(frame, name);

  if (frame == nullptr)
    ui->xyLabel->setText("N/A");
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

bool
SessionTabWidget::displayMeasurements() const
{
  return m_displayMeasurements;
}

void
SessionTabWidget::setDisplayMeasurements(bool disp)
{
  m_displayMeasurements = disp;
  m_glWidget->setDisplayMeasurements(disp);

  if (!disp)
    ui->xyLabel->setText("N/A");
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
  reconnectTracer();

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

  connect(
        ui->gridStepCombo,
        SIGNAL(activated(int)),
        this,
        SLOT(onGridStepChanged(int)));

  connect(
        ui->gridDivCombo,
        SIGNAL(activated(int)),
        this,
        SLOT(onGridDivChanged(int)));

    connect(
        m_glWidget,
        SIGNAL(planeCoords(qreal, qreal)),
        this,
        SLOT(onNewCoords(qreal, qreal)));
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

RZGUIGLWidget *
SessionTabWidget::glWidget() const
{
  return m_glWidget;
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

std::list<std::string>
SessionTabWidget::footprints() const
{
  std::list<std::string> footprints;

  for (auto &p : m_footprintQueues)
    footprints.push_back(p.first);

  for (auto &p : m_footprintWindows)
    footprints.push_back(p.first);

  return footprints;
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
SessionTabWidget::centerSelectedFrame()
{
  m_glWidget->rotateToCurrentFrame();
}

void
SessionTabWidget::updateDetectorWindow()
{
  m_detWindow->refreshImage();
}

void
SessionTabWidget::resetFootprintWindows()
{
  for (auto &p : m_footprintWindows) {
    p.second->close();
    p.second->deleteLater();
  }

  m_footprintWindows.clear();
}

SpotDiagramWindow *
SessionTabWidget::openNewFootprintWindow(std::string const &fullName)
{
  SpotDiagramWindow *window = nullptr;
  auto parts = fullName / ".";

  if (parts.size() != 2)
    return nullptr;

  // Get these pointers to inform the spot diagram window about the geometry
  RZ::OpticalElement *optEl = m_session->topLevelModel()->resolveOpticalElement(parts[0]);
  if (optEl == nullptr)
    return nullptr;

  const RZ::OpticalSurface *surf = optEl->opticalPath().getSurface(parts[1]);
  if (surf == nullptr)
    return nullptr;

  window = new SpotDiagramWindow(QString::fromStdString(parts[0]) + " on " + QString::fromStdString(parts[1]));

  auto it = m_footprintQueues.find(fullName);
  if (it != m_footprintQueues.end()) {
    auto &queue = m_footprintQueues[fullName];
    while (!queue.empty()) {
      window->transferFootprint(queue.front());
      queue.pop_front();
    }

    // Trigger recalc of the new footprints
    window->updateView();
    m_footprintQueues.erase(it);
  }

  m_footprintWindows[fullName] = window;

  return window;
}

void
SessionTabWidget::reloadModel()
{
  resetFootprintWindows();
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
  reconnectTracer();
}

void
SessionTabWidget::reconnectTracer()
{
  connect(
        m_session->tracer(),
        SIGNAL(aborted()),
        this,
        SIGNAL(simulationResults()));

  connect(
        m_session->tracer(),
        SIGNAL(error(QString)),
        this,
        SIGNAL(simulationResults()));

  connect(
        m_session->tracer(),
        SIGNAL(finished(bool)),
        this,
        SIGNAL(simulationResults()));
}

void
SessionTabWidget::reloadModelFromEditor()
{
  resetFootprintWindows();
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
  reconnectTracer();
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
  bool haveResults = false;

  m_progressDialog->simFinished();

  auto &footprints = m_session->state()->footprints();

  auto it = footprints.begin();
  while (it != footprints.end()) {
    auto curr = it++;

    // Non-opened window. Queue this result.
    if (m_footprintWindows.find(curr->fullName) == m_footprintWindows.end()) {
      auto &queue = m_footprintQueues[curr->fullName];
      queue.splice(queue.end(), footprints, curr);
    } else {
      // Opened window. Deliver it.
      auto window = m_footprintWindows[curr->fullName];
      window->transferFootprint(*curr);
      window->updateView();
    }

    haveResults = true;

  }

  footprints.clear();

  if (haveResults)
    emit simulationResults();
}

void
SessionTabWidget::onSourceEditorBuild()
{
  reloadModelFromEditor();
}

void
SessionTabWidget::onGridStepChanged(int ndx)
{
  if (ndx >= 0)
    setGridStep(ui->gridStepCombo->itemData(ndx).value<qreal>());
}

void
SessionTabWidget::onGridDivChanged(int ndx)
{
  if (ndx >= 0)
    setGridDivs(ui->gridDivCombo->itemData(ndx).value<unsigned>());
}

void
SessionTabWidget::onNewCoords(qreal x, qreal y)
{
  qreal theta = RZ::rad2deg(atan2(y, x));
  qreal length = sqrt(x * x + y * y);

  ui->xyLabel->setText(
   toSensibleUnits(x) + ", " + toSensibleUnits(y) + " (" + toSensibleUnits(length) + ", " + asScientific(theta) + "º)"
  );
}

void
SessionTabWidget::onOpenFootprintWindow()
{
  auto sender = qobject_cast<QAction *>(QObject::sender());
  SpotDiagramWindow *window = nullptr;

  if (sender != nullptr) {
    std::string fullName = sender->data().toString().toStdString();
    if (m_footprintWindows.find(fullName) == m_footprintWindows.end())
      window = openNewFootprintWindow(fullName);
    else
      window = m_footprintWindows[fullName];
  }

  if (window != nullptr)
    window->show();
}

void
SessionTabWidget::onCloseFootprintWindow()
{

}
