#include "SimulationPropertiesDialog.h"
#include "ui_SimulationPropertiesDialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QPushButton>
#include "PropertyAndDofExprModel.h"
#include "CustomTextEditDelegate.h"

SimulationPropertiesDialog::SimulationPropertiesDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SimulationPropertiesDialog)
{
  ui->setupUi(this);

  m_propModel = new PropertyAndDofExprModel(nullptr);
  ui->propView->setModel(m_propModel);

  ui->propView->setItemDelegateForColumn(
        3,
        new CustomTextEditDelegate(this));

  m_openSettingsDialog = new QFileDialog(this);
  m_saveSettingsDialog = new QFileDialog(this);

  m_openSettingsDialog->setWindowTitle("Load simulation settings");
  m_openSettingsDialog->setFileMode(QFileDialog::ExistingFile);
  m_openSettingsDialog->setAcceptMode(QFileDialog::AcceptOpen);
  m_openSettingsDialog->setNameFilter(
        "JSON simulation settings (*.json);;All files (*)");

  m_saveSettingsDialog->setWindowTitle("Export simulation settings");
  m_saveSettingsDialog->setFileMode(QFileDialog::AnyFile);
  m_saveSettingsDialog->setAcceptMode(QFileDialog::AcceptSave);
  m_saveSettingsDialog->setNameFilter(
        "JSON simulation settings (*.json);;All files (*)");

  connectAll();
}

SimulationPropertiesDialog::~SimulationPropertiesDialog()
{
  delete ui;
}

void
SimulationPropertiesDialog::connectAll()
{
  connect(
        ui->simTypeCombo,
        SIGNAL(activated(int)),
        this,
        SLOT(onDataChanged()));

  connect(
        ui->beamTypeCombo,
        SIGNAL(activated(int)),
        this,
        SLOT(onDataChanged()));

  connect(
        ui->apertureCombo,
        SIGNAL(activated(int)),
        this,
        SLOT(onDataChanged()));

  connect(
        ui->focalPlaneCombo,
        SIGNAL(activated(int)),
        this,
        SLOT(onDataChanged()));

  connect(
        ui->originCombo,
        SIGNAL(activated(int)),
        this,
        SLOT(onDataChanged()));

  connect(
        ui->diamEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onExprEditChanged()));

  connect(
        ui->fNumEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onExprEditChanged()));

  connect(
        ui->refApertureEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onExprEditChanged()));

  connect(
        ui->azimuthEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onExprEditChanged()));


  connect(
        ui->elevationEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onExprEditChanged()));


  connect(
        ui->offsetXEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onExprEditChanged()));


  connect(
        ui->offsetYEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onExprEditChanged()));

  connect(
        ui->loadSettingsButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onLoadSettings()));

  connect(
        ui->exportSettingsButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onExportSettings()));
}

void
SimulationPropertiesDialog::setSession(SimulationSession *session)
{
  m_session    = session;

  if (session != nullptr) {
    m_properties = session->state()->properties();
    m_propModel->setModel(session->topLevelModel());
    ui->modelNameLabel->setText(session->fileName());
    setWindowTitle("Simulation Properties - " + session->fileName());
  } else {
    m_propModel->setModel(nullptr);
    ui->modelNameLabel->setText("N/A");
    setWindowTitle("Simulation Properties (N/A)");
  }

  ui->propView->setModel(nullptr);
  ui->propView->setModel(m_propModel);

  ui->propView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  ui->propView->horizontalHeader()->setStretchLastSection(true);

  applyProperties();
}


void
SimulationPropertiesDialog::refreshUi()
{
  bool haveApertures   = ui->apertureCombo->count() > 0;
  bool haveFocalPlanes = ui->focalPlaneCombo->count() > 0;

  ui->steps1Label->setEnabled(m_properties.type != SIM_TYPE_ONE_SHOT);
  ui->steps2Label->setEnabled(m_properties.type == SIM_TYPE_2D_SWEEP);

  ui->steps1Spin->setEnabled(m_properties.type != SIM_TYPE_ONE_SHOT);
  ui->steps2Spin->setEnabled(m_properties.type == SIM_TYPE_2D_SWEEP);

  ui->fNumLabel->setEnabled(m_properties.beam  != BEAM_TYPE_COLLIMATED);
  ui->fNumEdit->setEnabled(m_properties.beam   != BEAM_TYPE_COLLIMATED);

  ui->refPlaneLabel->setEnabled(m_properties.beam    != BEAM_TYPE_COLLIMATED);
  ui->refApertureEdit->setEnabled(m_properties.beam  != BEAM_TYPE_COLLIMATED);

  switch (m_properties.ref) {
    case BEAM_REFERENCE_INPUT_ELEMENT:
      ui->refPlaneStack->setCurrentWidget(ui->inputElementPage);
      ui->refPlaneLabel->setText("Reference aperture");
      ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
      break;

    case BEAM_REFERENCE_APERTURE_STOP:
      ui->refPlaneStack->setCurrentWidget(ui->aperturePage);
      ui->refPlaneLabel->setText("Aperture stop");
      ui->refPlaneLabel->setEnabled(haveApertures);
      ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(haveApertures);
      break;

    case BEAM_REFERENCE_FOCAL_PLANE:
      ui->refPlaneStack->setCurrentWidget(ui->focalPlanePage);
      ui->refPlaneLabel->setText("Focal plane");
      ui->refPlaneLabel->setEnabled(haveFocalPlanes);
      ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(haveFocalPlanes);
      break;
  }

  ui->pathCombo->setEnabled(ui->pathCombo->count() > 0);
  ui->detectorCombo->setEnabled(ui->detectorCombo->count() > 0);
  ui->apertureCombo->setEnabled(haveApertures);
  ui->focalPlaneCombo->setEnabled(haveFocalPlanes);
}

void
SimulationPropertiesDialog::applyProperties(bool setEdited)
{
  ui->pathCombo->clear();
  ui->detectorCombo->clear();
  ui->apertureCombo->clear();
  ui->focalPlaneCombo->clear();

  BLOCKSIG(ui->simTypeCombo,    setCurrentIndex(m_properties.type));
  BLOCKSIG(ui->beamTypeCombo,   setCurrentIndex(m_properties.beam));
  BLOCKSIG(ui->originCombo,     setCurrentIndex(m_properties.ref));

  BLOCKSIG(ui->diamEdit,        setText(m_properties.diameter));
  BLOCKSIG(ui->fNumEdit,        setText(m_properties.fNum));
  BLOCKSIG(ui->refApertureEdit, setText(m_properties.refAperture));
  BLOCKSIG(ui->azimuthEdit,     setText(m_properties.azimuth));
  BLOCKSIG(ui->elevationEdit,   setText(m_properties.elevation));
  BLOCKSIG(ui->offsetXEdit,     setText(m_properties.offsetX));
  BLOCKSIG(ui->offsetYEdit,     setText(m_properties.offsetY));

  BLOCKSIG(ui->rayNumberSpin,   setValue(m_properties.rays));
  BLOCKSIG(ui->steps1Spin,      setValue(m_properties.Ni));
  BLOCKSIG(ui->steps2Spin,      setValue(m_properties.Nj));

  // Add all paths and detectors
  if (m_session != nullptr) {
    auto model = m_session->topLevelModel();
    auto paths = model->opticalPaths();
    auto dets  = model->detectors();
    auto stops = model->apertureStops();
    auto fps   = model->focalPlanes();

    // Add paths
    for (auto path : paths) {
      QString name = QString::fromStdString(path);
      QString prettyName = name;
      if (prettyName.size() == 0)
        prettyName = "(Default path)";

      ui->pathCombo->addItem(prettyName, name);
    }

    int index = ui->pathCombo->findData(m_properties.path);
    if (index == -1)
      index = 0;
    BLOCKSIG(ui->pathCombo, setCurrentIndex(index));

    if (paths.size() > 0) {
      // Add detectors
      ui->detectorCombo->addItem("(Path's default)", "");
      for (auto det : paths) {
        QString name = QString::fromStdString(det);
        ui->detectorCombo->addItem(name, name);
      }

      index = ui->detectorCombo->findData(m_properties.detector);
      if (index == -1)
        index = 0;
      BLOCKSIG(ui->detectorCombo, setCurrentIndex(index));
    }

    // Add apertures
    for (auto stop : stops) {
      QString name = QString::fromStdString(stop);
      ui->apertureCombo->addItem(name, name);
    }

    if (stops.size() > 0) {
      index = ui->apertureCombo->findData(m_properties.apertureStop);
      if (index == -1)
        index = 0;

      BLOCKSIG(ui->apertureCombo, setCurrentIndex(index));
    }

    // Add focal planes
    for (auto fp : fps) {
      QString name = QString::fromStdString(fp);
      ui->focalPlaneCombo->addItem(name, name);
    }

    if (fps.size() > 0) {
      index = ui->focalPlaneCombo->findData(m_properties.focalPlane);
      if (index == -1)
        index = 0;

      BLOCKSIG(ui->focalPlaneCombo, setCurrentIndex(index));
    }
  }

  for (auto p : m_properties.dofs)
    m_propModel->setDof(p.first, p.second, setEdited);

  refreshUi();
}

void
SimulationPropertiesDialog::parseProperties()
{
  switch (ui->simTypeCombo->currentIndex()) {
    case 0:
      m_properties.type = SIM_TYPE_ONE_SHOT;
      break;

    case 1:
      m_properties.type = SIM_TYPE_1D_SWEEP;
      break;

    case 2:
      m_properties.type = SIM_TYPE_2D_SWEEP;
      break;
  }

  switch (ui->beamTypeCombo->currentIndex()) {
    case 0:
      m_properties.beam = BEAM_TYPE_COLLIMATED;
      break;

    case 1:
      m_properties.beam = BEAM_TYPE_CONVERGING;
      break;

    case 2:
      m_properties.beam = BEAM_TYPE_DIVERGING;
      break;

  }

  switch (ui->originCombo->currentIndex()) {
    case 0:
      m_properties.ref = BEAM_REFERENCE_INPUT_ELEMENT;
      break;

    case 1:
      m_properties.ref = BEAM_REFERENCE_APERTURE_STOP;
      break;

    case 2:
      m_properties.ref = BEAM_REFERENCE_FOCAL_PLANE;
      break;
  }

  // Parse expressions
  m_properties.diameter     = ui->diamEdit->text();
  m_properties.fNum         = ui->fNumEdit->text();
  m_properties.refAperture  = ui->refApertureEdit->text();
  m_properties.azimuth      = ui->azimuthEdit->text();
  m_properties.elevation    = ui->elevationEdit->text();
  m_properties.offsetX      = ui->offsetXEdit->text();
  m_properties.offsetY      = ui->offsetYEdit->text();

  m_properties.focalPlane   = ui->focalPlaneCombo->currentData().toString();
  m_properties.apertureStop = ui->apertureCombo->currentData().toString();

  m_properties.rays         = ui->rayNumberSpin->value();
  m_properties.Ni           = ui->steps1Spin->value();
  m_properties.Nj           = ui->steps2Spin->value();

  if (ui->pathCombo->currentIndex() != -1)
    m_properties.path       = ui->pathCombo->currentData().value<QString>();
  else
    m_properties.path       = "";

  if (ui->detectorCombo->currentIndex() != -1)
    m_properties.detector   = ui->detectorCombo->currentData().value<QString>();
  else
    m_properties.detector   = "";

  m_properties.dofs.clear();
  for (auto p : m_session->topLevelModel()->dofs()) {
    if (m_propModel->dofEdited(p))
      m_properties.dofs[p] = m_propModel->dof(p);
  }
}

bool
SimulationPropertiesDialog::doUpdateState()
{
  bool stateUpdated = false;

  if (m_session != nullptr) {
    auto state = m_session->state();
    if (state->setProperties(m_properties)) {
      stateUpdated = true;
    } else {
      auto failed = state->getFirstInvalidExpr();
      QLineEdit *edit = nullptr;

      if (failed.size() != 0) {
        if (failed == "diameter")
          edit = ui->diamEdit;
        else if (failed == "fnum")
          edit = ui->fNumEdit;
        else if (failed == "refap")
          edit = ui->refApertureEdit;
        else if (failed == "azimuth")
          edit = ui->azimuthEdit;
        else if (failed == "elevation")
          edit = ui->elevationEdit;
        else if (failed == "offsetx")
          edit = ui->offsetXEdit;
        else if (failed == "offsety")
          edit = ui->offsetYEdit;
      }

      if (edit != nullptr) {
        QString asQString = QString::fromStdString(failed);
        edit->setStyleSheet("background-color: #ffbfbf");
        ui->tabWidget->setCurrentIndex(2);
        QMessageBox::critical(
              this,
              "Beam properties",
              "Expression for "
              + asQString
              + " contains errors: "
              + QString::fromStdString(state->getLastError()));
      } else {
        if (failed.substr(0, 4) == "dof:") {
          auto dofName = failed.substr(4);
          QString asQString = QString::fromStdString(dofName);
          m_propModel->setDofFailed(dofName);
          ui->tabWidget->setCurrentIndex(1);
          QMessageBox::critical(
                this,
                "Degree of freedom error",
                "Expression for degree of freedom `"
                + asQString
                + "' contains errors: "
                + QString::fromStdString(state->getLastError()));
        }
      }
    }
  }

  return stateUpdated;
}

void
SimulationPropertiesDialog::accept()
{
  parseProperties();

  if (doUpdateState())
    QDialog::accept();
}

bool
SimulationPropertiesDialog::doLoadFromFile()
{
  bool opened = false;
  if (m_session == nullptr)
    return false;

  try {
    if (m_openSettingsDialog->exec()
        && !m_openSettingsDialog->selectedFiles().empty()) {
      QString fileName = m_openSettingsDialog->selectedFiles()[0];
      QFile   file(fileName);

      if (!file.open(QIODevice::ReadOnly))
        throw std::runtime_error(
            "Cannot load simulation settings from the selected file: "
            + file.errorString().toStdString());

      if (file.size() > MAX_SIMULATION_CONFIG_FILE_SIZE)
        throw std::runtime_error(
            "Settings file is too big (probably not a settings file)");

      auto data = file.readAll();
      if (file.error() != QFileDevice::NoError)
        throw std::runtime_error(
            "Read error while loading settings: "
            + file.errorString().toStdString());

      SimulationProperties properties;
      if (!properties.deserialize(data))
        throw std::runtime_error(
            "Simulation file contains errors: "
            + properties.lastError().toStdString());

      m_properties = properties;
      m_propModel->setModel(m_session->topLevelModel());
      applyProperties(true);

      ui->propView->setModel(nullptr);
      ui->propView->setModel(m_propModel);

      ui->propView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
      ui->propView->horizontalHeader()->setStretchLastSection(true);

      opened = true;
    }
  } catch (std::runtime_error const &e) {
    QString error = QString(e.what());
    QMessageBox::critical(
          this,
          "Load simulation settings",
          error);
  }

  return opened;
}

////////////////////////////////////// Slots ///////////////////////////////////
void
SimulationPropertiesDialog::onDataChanged()
{
  parseProperties();
  refreshUi();
}

void
SimulationPropertiesDialog::onExprEditChanged()
{
  auto sender = static_cast<QLineEdit *>(QObject::sender());

  sender->setStyleSheet("");
}

void
SimulationPropertiesDialog::onLoadSettings()
{
  doLoadFromFile();
}

void
SimulationPropertiesDialog::onExportSettings()
{
  parseProperties();

  if (m_saveSettingsDialog->exec()
      && !m_saveSettingsDialog->selectedFiles().empty()) {
    QString fileName = m_saveSettingsDialog->selectedFiles()[0];
    auto    props    = m_properties.serialize();
    QFile   file(fileName);

    if (file.open(QIODevice::WriteOnly)) {
      auto ret = file.write(props);
      if (file.error() != QFileDevice::NoError) {
        QMessageBox::critical(
              this,
              "Export simulation settings",
              "Write error while saving settings: " + file.errorString());
      }
    } else {
      QMessageBox::critical(
            this,
            "Export simulation settings",
            "Cannot export simulation settings to the selected file: "
            + file.errorString());
    }
  }
}
