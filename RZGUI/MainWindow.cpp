#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "SimulationSession.h"
#include "SessionTabWidget.h"
#include "PropertyAndDofTableModel.h"
#include "OMTreeModel.h"
#include "SimulationPropertiesDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSurfaceFormat>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  m_propModel           = new PropertyAndDofTableModel(nullptr);
  m_omModel             = new OMTreeModel();
  m_simPropertiesDialog = new SimulationPropertiesDialog(this);

  ui->propTableView->setModel(m_propModel);
  ui->omTreeView->setModel(m_omModel);

  QSurfaceFormat fmt;
  fmt.setSamples(2);
  QSurfaceFormat::setDefaultFormat(fmt);

  connectAll();
  refreshCurrentSession();
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
        ui->actionRunSim,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onSimulationRun()));

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
        ui->actionToggleDisplayNames,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onChangeDisplay()));
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
MainWindow::refreshCurrentSession()
{
  if (m_currSession != nullptr) {
    // Refresh model, if applicable
    m_propModel->setModel(m_currSession->topLevelModel());
    m_omModel->setModel(m_currSession->topLevelModel());
    m_simPropertiesDialog->setSession(m_currSession);

    ui->animationToolBar->setEnabled(true);
    ui->viewToolBar->setEnabled(true);
    ui->actionAnimPause->setEnabled(m_currSession->playing());
    ui->actionAnimStop->setEnabled(!m_currSession->stopped());
    ui->actionAnimPlay->setEnabled(!m_currSession->playing());

    ui->simToolBar->setEnabled(true);
    ui->actionSimResult->setEnabled(
          m_currSession->topLevelModel()->detectors().size() > 0);
    setWindowTitle("RayZaler - " + m_currSession->fileName());

    ui->displayToolBar->setEnabled(true);
    BLOCKSIG(
          ui->actionToggleDisplayNames,
          setChecked(m_sessionToTab[m_currSession]->displayNames()));

    reconnectModels();
  } else {
    m_propModel->setModel(nullptr);
    m_omModel->setModel(nullptr);
    m_simPropertiesDialog->setSession(nullptr);

    ui->animationToolBar->setEnabled(false);
    ui->viewToolBar->setEnabled(false);
    ui->simToolBar->setEnabled(false);
    ui->displayToolBar->setEnabled(false);
    BLOCKSIG(ui->actionToggleDisplayNames, setChecked(false));
    ui->propTableView->setModel(nullptr);
    setWindowTitle("RayZaler - No model file");
  }
}

void
MainWindow::registerSession(SimulationSession *session)
{
  SessionTabWidget *widget = new SessionTabWidget(session);

  m_sessions.push_back(session);
  m_sessionToTab.insert(session, widget);

  ui->sessionTabWidget->addTab(widget, session->fileName());
  ui->sessionTabWidget->setCurrentWidget(widget);

  m_currSession = session;
  refreshCurrentSession();
}

void
MainWindow::doOpen()
{
  QFileDialog dialog(this);
  QStringList filters;
  bool done = false;


  filters <<
             "RayZaler model files (*.rzm)" <<
             "All files (*)";

  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setNameFilters(filters);

  do {
    if (dialog.exec()) {
      auto files = dialog.selectedFiles();
      if (files.size() > 0) {
        auto firstFile = files[0];
        try {
          registerSession(new SimulationSession(firstFile, this));
          done = true;
        } catch (std::runtime_error &e) {
          QMessageBox::critical(
                this,
                "Load model file",
                QString::fromStdString(e.what()));
          done = false;
        }
      } else {
        done = true;
      }
    } else {
      done = true;
    }
  } while (!done);
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
MainWindow::onCloseTab(int index)
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->widget(index));

  SimulationSession *session = widget->session();

  ui->sessionTabWidget->removeTab(index);

  m_sessions.remove(session);
  m_sessionToTab.remove(session);

  delete widget;
  delete session;
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
              "Simulation failed. See log window for details");
      }
    }
  }
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

  ui->propTableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
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
    ROTONSENDER(actionViewRear,   180,   0, 0)
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
    QModelIndex index = ui->omTreeView->currentIndex();
    RZ::Element *selectedElement = nullptr;
    auto item = m_omModel->itemFromIndex(index);

    if (item != nullptr) {
      if (item->type == OM_TREE_ITEM_TYPE_ELEMENT
          || item->type == OM_TREE_ITEM_TYPE_OPTICAL_ELEMENT
          || item->type == OM_TREE_ITEM_TYPE_DETECTOR) {
        selectedElement = item->element;
      }
    }

    m_currSession->selectElement(selectedElement);
  }
}

void
MainWindow::onChangeDisplay()
{
  SessionTabWidget *widget = qobject_cast<SessionTabWidget *>(
        ui->sessionTabWidget->currentWidget());

  if (widget != nullptr)
    widget->setDisplayNames(ui->actionToggleDisplayNames->isChecked());
}

