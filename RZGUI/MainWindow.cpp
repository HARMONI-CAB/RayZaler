#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "SimulationSession.h"
#include "SessionTabWidget.h"
#include "PropertyAndDofTableModel.h"
#include "OMTreeModel.h"
#include "SimulationPropertiesDialog.h"
#include "CustomTextEditDelegate.h"
#include "ElementPropertyModel.h"
#include "DOFWidget.h"
#include "AboutDialog.h"
#include "ExportViewDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSurfaceFormat>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  m_propModel           = new PropertyAndDofTableModel(nullptr);
  m_compPropModel       = new ElementPropertyModel;
  m_omModel             = new OMTreeModel;
  m_simPropertiesDialog = new SimulationPropertiesDialog(this);
  m_exportViewDialog    = new ExportViewDialog(this);
  m_aboutDialog         = new AboutDialog(this);

  ui->propTableView->setModel(m_propModel);
  ui->propTableView->setItemDelegateForColumn(
        3,
        new CustomTextEditDelegate(this));

  ui->compPropView->setModel(m_compPropModel);
  ui->omTreeView->setModel(m_omModel);

  QSurfaceFormat fmt;
  fmt.setSamples(2);
  QSurfaceFormat::setDefaultFormat(fmt);

  connectAll();
  refreshCurrentSession();

  RZ::Logger::setDefaultLogger(this);

  RZ::RZInit();
}

void
MainWindow::logFunction(
        RZ::LogLevel level,
        std::string const &file,
        int line,
        std::string const &message)
{
  QString text, prefix;
  QFileInfo info(QString::fromStdString(file));
  QString baseName = info.fileName();
  text = QString::fromStdString(message).replace("\n", "<br />\n");

  switch (level) {
    case RZ::LOG_ERROR:
        prefix =
            "<b><font color = \"red\">Error </font>("
            + baseName
            + ":"
            + QString::number(line)
            + ")</b>: ";
      break;

    case RZ::LOG_WARNING:
      prefix =
          "<b><font color = \"#a3732f\">Warning </font>("
          + baseName
          + ":"
          + QString::number(line)
          + ")</b>: ";
      break;

    case RZ::LOG_INFO:
      prefix = "<b>Info</b>: ";
      break;
  }

  QTextCursor cursor = ui->logTextEdit->textCursor();

  cursor.movePosition(QTextCursor::End);
  ui->logTextEdit->setTextCursor(cursor);
  ui->logTextEdit->insertHtml(prefix + text);
  cursor.movePosition(QTextCursor::End);
  ui->logTextEdit->setTextCursor(cursor);
}

void
MainWindow::connectAll()
{
  connect(
        ui->actionOpen,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onOpen()));

  connect(
        ui->actionReloadModel,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onReload()));


  connect(
        ui->sessionTabWidget,
        SIGNAL(tabCloseRequested(int)),
        this,
        SLOT(onCloseTab(int)));

  connect(
        ui->sessionTabWidget,
        SIGNAL(currentChanged(int)),
        this,
        SLOT(onTabChanged()));

  connect(
        m_propModel,
        SIGNAL(modelReset()),
        this,
        SLOT(onModelsChanged()));

  connect(
        m_propModel,
        SIGNAL(dataChanged(QModelIndex, QModelIndex, QList<int>)),
        this,
        SLOT(onDofChanged(QModelIndex, QModelIndex, QList<int>)));

  connect(
        m_compPropModel,
        SIGNAL(propertyChanged(QString)),
        this,
        SLOT(onUpdateModel()));

  connect(
        m_omModel,
        SIGNAL(modelReset()),
        this,
        SLOT(onModelsChanged()));

  connect(
        ui->actionAnimStart,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onAnimStart()));

  connect(
        ui->actionAnimStop,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onAnimStop()));

  connect(
        ui->actionAnimPlay,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onAnimPlay()));

  connect(
        ui->actionAnimPause,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onAnimPause()));

  connect(
        ui->actionAnimEnd,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onAnimEnd()));

  connect(
        ui->actionSimProp,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onSimulationEditProperties()));

  connect(
        ui->actionRunSimFile,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onSimulationLoadAndRun()));

  connect(
        ui->actionExportView,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onExportImage()));

  connect(
        ui->actionRunSim,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onSimulationRun()));

  connect(
        ui->actionClearBeam,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onClearBeam()));

  connect(
        ui->actionSimResult,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onSimulationShowResult()));

  connect(
        ui->actionViewTop,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onChangeView()));

  connect(
        ui->actionViewBottom,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onChangeView()));

  connect(
        ui->actionViewFront,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onChangeView()));

  connect(
        ui->actionViewRear,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onChangeView()));

  connect(
        ui->actionViewLeft,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onChangeView()));

  connect(
        ui->actionViewRight,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onChangeView()));

  connect(
        ui->actionCenterSelection,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onCenterToSelected()));

  connect(
        ui->actionToggleDisplayNames,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onChangeDisplay()));

  connect(
        ui->actionToggleApertures,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onChangeDisplay()));

  connect(
        ui->actionToggleElements,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onChangeDisplay()));

  connect(
        ui->actionToggleReferenceFrames,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onChangeDisplay()));

  connect(
        ui->actionToggleGrid,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onChangeDisplay()));

  connect(
        ui->actionToggleMeasurements,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onChangeDisplay()));
        
  connect(
        ui->actionModelSource,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onModelSource()));

  connect(
        ui->actionToggleCurrent,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onToggleCurrent()));
  
  connect(
        ui->action_About_RZGUI,
        SIGNAL(triggered(bool)),
        m_aboutDialog,
        SLOT(exec()));

  connect(
        this,
        SIGNAL(ready()),
        this,
        SLOT(onShow()));
}

void
MainWindow::pushDelayedOpenFile(QString file)
{
  m_delayedFiles.push_back(file);
}

void
MainWindow::openDelayedFiles()
{
  for (auto &f : m_delayedFiles)
    (void) openModelFile(f);
}

void
MainWindow::notifyReady()
{
  emit ready();
}

void
MainWindow::reconnectModels()
{
  connect(
        ui->omTreeView->selectionModel(),
        SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
        this,
        SLOT(onTreeItemSelectionChanged()));
}

void
MainWindow::keyPressEvent(QKeyEvent *event)
{
  if (m_currSession != nullptr && m_sessionToUi.contains(m_currSession)) {
    auto widget = m_sessionToUi[m_currSession].tab;

    if (widget != nullptr)
      widget->keyPressEvent(event);
  }

  QWidget::keyPressEvent(event);
}

void
MainWindow::refreshCurrentElement()
{
  RZ::Element *element = nullptr;

  if (m_currSession != nullptr)
    element = m_currSession->getSelectedElement();

  m_compPropModel->setElement(element);

  ui->compPropView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
  ui->compPropView->horizontalHeader()->setStretchLastSection(true);
}

void
MainWindow::refreshCurrentSession()
{
  if (ui->dofStack->count() > 1)
    ui->dofStack->removeWidget(ui->dofStack->widget(1));

  if (m_currSession != nullptr) {
    // Refresh model, if applicable
    m_propModel->setModel(m_currSession->topLevelModel());
    m_omModel->setModel(m_currSession->topLevelModel());
    m_simPropertiesDialog->setSession(m_currSession);

    ui->animationToolBar->setEnabled(true);
    ui->viewToolBar->setEnabled(true);
    ui->actionReloadModel->setEnabled(true);
    ui->actionAnimPause->setEnabled(m_currSession->playing());
    ui->actionAnimStop->setEnabled(!m_currSession->stopped());
    ui->actionAnimPlay->setEnabled(!m_currSession->playing());
    ui->actionModelSource->setEnabled(true);
    ui->dofStack->insertWidget(1, m_sessionToUi[m_currSession].dofWidget);
    ui->dofStack->setCurrentIndex(1);
    ui->actionExportView->setEnabled(true);

    SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
          ui->sessionTabWidget->currentWidget());
    if (widget != nullptr)
      ui->actionCenterSelection->setEnabled(widget->selectedFrame() != nullptr);

    ui->simToolBar->setEnabled(true);
    ui->actionSimResult->setEnabled(
          m_currSession->topLevelModel()->detectors().size() > 0);
    setWindowTitle("RayZaler - " + m_currSession->fileName());

    ui->displayToolBar->setEnabled(true);
    BLOCKSIG(
          ui->actionToggleDisplayNames,
          setChecked(m_sessionToUi[m_currSession].tab->displayNames()));

    BLOCKSIG(
          ui->actionToggleApertures,
          setChecked(m_sessionToUi[m_currSession].tab->displayApertures()));

    BLOCKSIG(
          ui->actionToggleElements,
          setChecked(m_sessionToUi[m_currSession].tab->displayElements()));

    BLOCKSIG(
          ui->actionToggleReferenceFrames,
          setChecked(m_sessionToUi[m_currSession].tab->displayRefFrames()));

    BLOCKSIG(
          ui->actionToggleGrid,
          setChecked(m_sessionToUi[m_currSession].tab->displayGrid()));

    BLOCKSIG(
          ui->actionToggleMeasurements,
          setChecked(m_sessionToUi[m_currSession].tab->displayMeasurements()));

    reconnectModels();
  } else {
    m_propModel->setModel(nullptr);
    m_omModel->setModel(nullptr);
    m_simPropertiesDialog->setSession(nullptr);

    ui->animationToolBar->setEnabled(false);
    ui->viewToolBar->setEnabled(false);
    ui->simToolBar->setEnabled(false);
    ui->displayToolBar->setEnabled(false);
    ui->actionReloadModel->setEnabled(false);
    ui->actionModelSource->setEnabled(false);
    ui->actionExportView->setEnabled(false);

    BLOCKSIG(ui->actionToggleDisplayNames,    setChecked(false));
    BLOCKSIG(ui->actionToggleApertures,       setChecked(false));
    BLOCKSIG(ui->actionToggleElements,        setChecked(true));
    BLOCKSIG(ui->actionToggleReferenceFrames, setChecked(false));
    BLOCKSIG(ui->actionToggleGrid,            setChecked(true));
    BLOCKSIG(ui->actionToggleMeasurements,    setChecked(false));

    ui->propTableView->setModel(nullptr);
    setWindowTitle("RayZaler - No model file");
  }

  refreshCurrentElement();
}

void
MainWindow::registerSession(SimulationSession *session)
{
  SessionUI sessUi;

  sessUi.tab = new SessionTabWidget(session);
  
  initDOFWidget(sessUi, session);

  m_sessions.push_back(session);
  m_sessionToUi.insert(session, sessUi);

  ui->sessionTabWidget->addTab(sessUi.tab, session->fileName());
  ui->sessionTabWidget->setCurrentWidget(sessUi.tab);

  m_currSession = session;
  refreshCurrentSession();
}

bool
MainWindow::openModelFile(QString file)
{
  bool ok = false;

  QFileInfo info(file);
  m_lastOpenDir = info.dir().path();
  
  try {
    registerSession(new SimulationSession(file, this));
    ok = true;
  } catch (std::runtime_error &e) {
    QMessageBox::critical(
          this,
          "Load model file",
          QString::fromStdString(e.what()));
  }

  return ok;
}

void
MainWindow::doOpen()
{
  bool done = false;
  QStringList filters;

  filters <<
              "RayZaler model files (*.rzm)" <<
              "All files (*)";

  if (m_lastOpenDir.size() == 0)
    m_lastOpenDir = QDir::currentPath();
  
  do {
    QFileDialog openDialog;
    openDialog.setFileMode(QFileDialog::ExistingFile);
    openDialog.setNameFilters(filters);
    openDialog.setDirectory(m_lastOpenDir);

    if (openDialog.exec()) {
      auto files = openDialog.selectedFiles();
      if (files.size() > 0) {
        done = false;
        for (auto &e : files)
          done = openModelFile(e) || done;
      } else {
        done = true;
      }
    } else {
      done = true;
    }
  } while (!done);
}

void
MainWindow::initDOFWidget(SessionUI &sessUI, SimulationSession *sess)
{
  sessUI.dofWidget = new DOFWidget(sess);
  connect(
      sessUI.dofWidget,
      SIGNAL(dofChanged()),
      this,
      SLOT(onUpdateModel()));
}

void
MainWindow::finalizeDOFWidget(SessionUI &sessUI)
{
  sessUI.dofWidget->deleteLater();
  sessUI.dofWidget = nullptr;
}

void
MainWindow::doReload()
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->currentWidget());

  if (widget != nullptr) {
    // We need to remove the DOF widget from the current session UI, 
    // as it will refer to objects that will no longer exist after the reload.

    auto current = m_currSession;
    auto &sessUI = m_sessionToUi[current];
    finalizeDOFWidget(sessUI);

    m_currSession = nullptr;
    refreshCurrentSession();

    widget->reloadModel();

    // Add the up-to-date DOF widget.
    initDOFWidget(sessUI, current);
    
    m_currSession = current;
    refreshCurrentSession();
  }
}

MainWindow::~MainWindow()
{
  delete ui;
}

void
MainWindow::onOpen()
{
  doOpen();
}

void
MainWindow::onShow()
{
  openDelayedFiles();
}

void
MainWindow::onReload()
{
  doReload();
}

void
MainWindow::onCloseTab(int index)
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->widget(index));

  SimulationSession *session = widget->session();

  auto sessUI = m_sessionToUi[session];

  ui->sessionTabWidget->removeTab(index);

  m_sessions.remove(session);
  m_sessionToUi.remove(session);

  sessUI.tab->deleteLater();
  finalizeDOFWidget(sessUI);

  session->deleteLater();
}

void
MainWindow::onTabChanged()
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->currentWidget());

  if (widget != nullptr) {
    m_currSession = widget->session();
  } else {
    m_currSession = nullptr;
  }

  refreshCurrentSession();
}

void
MainWindow::onAnimStart()
{
  m_currSession->animBegin();
  refreshCurrentSession();
}

void
MainWindow::onAnimEnd()
{
  m_currSession->animEnd();
  refreshCurrentSession();
}

void
MainWindow::onAnimPause()
{
  m_currSession->animPause();
  refreshCurrentSession();
}

void
MainWindow::onAnimPlay()
{
  m_currSession->animPlay();
  refreshCurrentSession();
}

void
MainWindow::onAnimStop()
{
  m_currSession->animStop();
  refreshCurrentSession();
}

void
MainWindow::onSimulationEditProperties()
{
  if (m_currSession != nullptr)
    m_simPropertiesDialog->exec();
}

void
MainWindow::onSimulationLoadAndRun()
{
  if (m_simPropertiesDialog->doLoadFromFile()) {
    // The properties have been loaded, but now we have to apply them
    // to the current simulation state.

    if (!m_simPropertiesDialog->doUpdateState()) {
      // Failed to update state. Query the user about the right settings
      if (m_simPropertiesDialog->exec() != QDialog::Accepted)
        return;
    }

    if (m_currSession->state()->canRun()) {
      if (!m_currSession->runSimulation()) {
        QMessageBox::critical(
              this,
              "Simulation error",
              "Simulation failed. "
              + QString::fromStdString(m_currSession->state()->getLastError()));
      }
    }
  }
}

void
MainWindow::onSimulationRun()
{
  if (m_currSession != nullptr) {
    if (!m_currSession->state()->canRun())
      m_simPropertiesDialog->exec();

    if (m_currSession->state()->canRun()) {
      if (!m_currSession->runSimulation()) {
        QMessageBox::critical(
              this,
              "Simulation error",
              "Simulation failed. "
              + QString::fromStdString(m_currSession->state()->getLastError()));
      }
    }
  }
}

void
MainWindow::onClearBeam()
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->currentWidget());

  if (widget != nullptr)
    widget->clearBeam();
}

void
MainWindow::onSimulationShowResult()
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->currentWidget());

  if (widget != nullptr)
    widget->showDetectorWindow();
}

void
MainWindow::onModelsChanged()
{
  ui->propTableView->setModel(nullptr);
  ui->propTableView->setModel(m_propModel);

  ui->propTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  ui->propTableView->horizontalHeader()->setStretchLastSection(true);

  ui->omTreeView->setModel(nullptr);
  ui->omTreeView->setModel(m_omModel);
}

void
MainWindow::onDofChanged(
    const QModelIndex &,
    const QModelIndex &,
    const QList<int> &)
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->currentWidget());

  if (widget != nullptr)
    widget->updateModel();
}

void
MainWindow::onChangeView()
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->currentWidget());

  if (widget != nullptr) {
    QAction *sender = qobject_cast<QAction *>(QObject::sender());

#define ROTONSENDER(act, x, y, z) \
  if (sender == ui->act) {        \
    widget->setRotation(x, y, z); \
    return;                       \
  }

    ROTONSENDER(actionViewFront,  0,     0, 0)
    ROTONSENDER(actionViewRear,  -180,   0, 0)
    ROTONSENDER(actionViewTop,      0, +90, 0)
    ROTONSENDER(actionViewBottom,   0, -90, 0)
    ROTONSENDER(actionViewLeft,   +90,   0, 0)
    ROTONSENDER(actionViewRight,  -90,   0, 0)
  }

#undef ROTONSENDER
}

void
MainWindow::onTreeItemSelectionChanged()
{
  if (m_currSession != nullptr) {
    SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->currentWidget());
    QModelIndex index = ui->omTreeView->currentIndex();
    RZ::Element *selectedElement = nullptr;
    std::string displayText;
    const char *name = nullptr;
    RZ::ReferenceFrame *selectedFrame = nullptr;
    const RZ::OpticalPath *selectedPath = nullptr;

    auto item = m_omModel->itemFromIndex(index);

    if (item != nullptr) {
      if (item->type == OM_TREE_ITEM_TYPE_ELEMENT
          || item->type == OM_TREE_ITEM_TYPE_OPTICAL_ELEMENT
          || item->type == OM_TREE_ITEM_TYPE_DETECTOR) {
        selectedElement = item->element;
      } else if (item->type == OM_TREE_ITEM_TYPE_FRAME) {
        selectedFrame = item->frame;
        displayText   = item->displayText.toStdString();
        name          = displayText.c_str();
      } else if (item->type == OM_TREE_ITEM_TYPE_OPTICAL_PATH) {
        selectedPath = item->path;
      }
    }

    ui->actionCenterSelection->setEnabled(widget->selectedFrame() != nullptr);
    m_currSession->selectElement(selectedElement);
    widget->setSelectedReferenceFrame(selectedFrame, name);
    widget->setSelectedOpticalPath(selectedPath);
  }

  refreshCurrentElement();
}

void
MainWindow::onChangeDisplay()
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->currentWidget());

  if (widget != nullptr) {
    widget->setDisplayNames(ui->actionToggleDisplayNames->isChecked());
    widget->setDisplayApertures(ui->actionToggleApertures->isChecked());
    widget->setDisplayElements(ui->actionToggleElements->isChecked());
    widget->setDisplayRefFrames(ui->actionToggleReferenceFrames->isChecked());
    widget->setDisplayGrid(ui->actionToggleGrid->isChecked());
    widget->setDisplayMeasurements(ui->actionToggleMeasurements->isChecked());
  }
}

void
MainWindow::onToggleCurrent()
{
  if (m_currSession != nullptr)
    m_currSession->toggleCurrent();
}

void
MainWindow::onUpdateModel()
{
  if (m_currSession != nullptr) {
    auto &ui = m_sessionToUi[m_currSession];
    ui.tab->updateModel();
  }
}

void
MainWindow::onModelSource()
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->currentWidget());

  if (widget != nullptr)
    widget->showSourceWindow();
}

void
MainWindow::onCenterToSelected()
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->currentWidget());

  if (widget != nullptr)
    widget->centerSelectedFrame();
}

void
MainWindow::onExportImage()
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->currentWidget());

  if (widget != nullptr) {
    m_exportViewDialog->setSessionTabWidget(widget);
    m_exportViewDialog->setModal(true);
    m_exportViewDialog->show();
  }
}
