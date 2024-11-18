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

#include "SimulationPropertiesDialog.h"
#include "ui_SimulationPropertiesDialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QPushButton>
#include <QFileInfo>
#include "PropertyAndDofExprModel.h"
#include "CustomTextEditDelegate.h"
#include "ColorChooserButton.h"
#include "BeamPropertiesDialog.h"
#include "RZGUI.h"
#include <QJsonDocument>

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

  m_fixedColorChooser = new ColorChooserButton();
  m_fixedColorChooser->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

  ui->colorCycleLayout->addWidget(m_fixedColorChooser, 0, 2, 1, 1);

  auto spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  ui->colorCycleLayout->addItem(spacer, 0, 3, 1, 1);

  m_beamPropertiesDialog = new BeamPropertiesDialog(nullptr, this);

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

  applySettings();
}

SimulationPropertiesDialog::~SimulationPropertiesDialog()
{
  delete ui;
}

void
SimulationPropertiesDialog::refreshBeamList()
{
  ui->beamTableWidget->setRowCount(0);

  printf("Beams in beam vector: %d\n", m_properties.beamVector.size());

  for (int i = 0; i < m_properties.beamVector.size(); ++i) {
    QString shape, sampling;

    if (i != ui->beamTableWidget->rowCount())
      throw std::runtime_error("Unexpected beamTableWidget behavior");

    int row = i;

    ui->beamTableWidget->insertRow(row);

    auto props = m_properties.beamVector[i];

    switch (props->shape) {
      case RZ::BeamShape::Circular:
        shape = "Circular";
        break;

      case RZ::BeamShape::Ring:
        shape = "Ring";
        break;

      case RZ::BeamShape::Point:
        shape = "Point-like";
        break;

      case RZ::BeamShape::Custom:
        shape = "Custom";
        break;
    }

    sampling = props->random ? "Random" : "Uniform";

    auto item = new QTableWidgetItem;

    item->setBackground(props->color);

    ui->beamTableWidget->setItem(row, 0, item);
    ui->beamTableWidget->setItem(row, 1, new QTableWidgetItem(props->name));
    ui->beamTableWidget->setItem(row, 2, new QTableWidgetItem(shape));
    ui->beamTableWidget->setItem(row, 3, new QTableWidgetItem(props->fNum));
    ui->beamTableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(props->rays)));
    ui->beamTableWidget->setItem(row, 5, new QTableWidgetItem(sampling));
  }
}

void
SimulationPropertiesDialog::applySettings()
{
  RZGUISingleton::loadSetting(m_repProperties, "Representation");
  QRadioButton *coloringRadio = nullptr;

  BLOCKSIG(ui->accumCheck, setChecked(m_repProperties.accumulate));
  BLOCKSIG(m_fixedColorChooser, setColor(m_repProperties.fixedBeamColor));

  switch (m_repProperties.coloringMode) {
    case COLORING_FIXED:
      coloringRadio = ui->fixedColorRadio;
      break;

    case COLORING_WAVELENGTH:
      coloringRadio = ui->wlColorRadio;
      break;

    case COLORING_CYCLE:
      coloringRadio = ui->incrementalColorRadio;
      break;
  }

  if (coloringRadio != nullptr)
    BLOCKSIG(coloringRadio, setChecked(true));
}

void
SimulationPropertiesDialog::connectAll()
{
  connect(
        ui->addBeamButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onAddBeam()));

  connect(
        ui->removeBeamButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onRemoveBeam()));

  connect(
        ui->removeAllBeamsButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onRemoveAllBeams()));

  connect(
        ui->tracingType,
        SIGNAL(activated(int)),
        this,
        SLOT(onDataChanged()));

  connect(
        ui->simTypeCombo,
        SIGNAL(activated(int)),
        this,
        SLOT(onDataChanged()));


  connect(
        ui->saveCheck,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onDataChanged()));

  connect(
        ui->browseDirButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onBrowseOutputDir()));

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

  connect(
        ui->fixedColorRadio,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onRepChanged()));

  connect(
        ui->addFootprintButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onAddFootprint()));

  connect(
        ui->removeFootprintButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onRemoveFootprint()));

  connect(
        ui->removeAllFootprintsButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onRemoveAllFootprints()));

  connect(
        ui->beamTableWidget,
        SIGNAL(cellDoubleClicked(int, int)),
        this,
        SLOT(onEditBeam(int, int)));

  connect(
        ui->beamTableWidget,
        SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *)),
        this,
        SLOT(refreshUi()));

  connect(
        ui->removeBeamButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onRemoveBeam()));

  connect(
        ui->removeAllBeamsButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onRemoveAllBeams()));
}

void
SimulationPropertiesDialog::setSession(SimulationSession *session)
{
  m_session    = session;
  m_beamPropertiesDialog->setSession(session);

  ui->opticalElementCombo->clear();

  if (session != nullptr) {
    auto topLevel = session->topLevelModel();
    m_properties = session->state()->properties();
    m_propModel->setModel(topLevel);
    ui->modelNameLabel->setText(session->fileName());
    setWindowTitle("Simulation Properties - " + session->fileName());
  } else {
    m_properties = SimulationProperties();
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
  ui->steps1Label->setEnabled(m_properties.type != SIM_TYPE_ONE_SHOT);
  ui->steps2Label->setEnabled(m_properties.type == SIM_TYPE_2D_SWEEP);

  ui->steps1Spin->setEnabled(m_properties.type != SIM_TYPE_ONE_SHOT);
  ui->steps2Spin->setEnabled(m_properties.type == SIM_TYPE_2D_SWEEP);

  m_fixedColorChooser->setEnabled(ui->fixedColorRadio->isChecked());

  ui->pathCombo->setEnabled(ui->pathCombo->count() > 0);
  ui->detectorCombo->setEnabled(ui->detectorCombo->count() > 0);

  ui->saveCheck->setEnabled(ui->detectorSaveCombo->count());
  if (!ui->saveCheck->isEnabled())
    BLOCKSIG(ui->saveCheck, setChecked(false));

  ui->detectorSaveCombo->setEnabled(ui->saveCheck->isChecked());
  ui->saveCSVCheck->setEnabled(ui->saveCheck->isChecked());
  ui->outputDirEdit->setEnabled(ui->saveCheck->isChecked());
  ui->outputDirLabel->setEnabled(ui->saveCheck->isChecked());
  ui->clearDetCheck->setEnabled(ui->saveCheck->isChecked());
  ui->browseDirButton->setEnabled(ui->saveCheck->isChecked());
  ui->overwriteResultsCheck->setEnabled(ui->saveCheck->isChecked());
  ui->removeBeamButton->setEnabled(ui->beamTableWidget->currentRow() != -1);
  ui->removeAllBeamsButton->setEnabled(!m_properties.beams.empty());
}

void
SimulationPropertiesDialog::applyProperties(bool setEdited)
{
  ui->pathCombo->clear();
  ui->detectorCombo->clear();
  ui->detectorSaveCombo->clear();
  ui->opticalElementCombo->clear();

  BLOCKSIG(ui->tracingType,           setCurrentIndex(m_properties.ttype));
  BLOCKSIG(ui->simTypeCombo,          setCurrentIndex(m_properties.type));

  BLOCKSIG(ui->steps1Spin,            setValue(m_properties.Ni));
  BLOCKSIG(ui->steps2Spin,            setValue(m_properties.Nj));

  BLOCKSIG(ui->saveCheck,             setChecked(m_properties.saveArtifacts));
  BLOCKSIG(ui->saveCSVCheck,          setChecked(m_properties.saveCSV));
  BLOCKSIG(ui->clearDetCheck,         setChecked(m_properties.clearDetector));
  BLOCKSIG(ui->overwriteResultsCheck, setChecked(m_properties.overwrite));
  BLOCKSIG(ui->outputDirEdit,         setText(m_properties.saveDir));

  // Add all beams
  refreshBeamList();

  // Add all paths and detectors
  if (m_session != nullptr) {
    auto model = m_session->topLevelModel();
    auto paths = model->opticalPaths();
    auto dets  = model->detectors();

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

    if (dets.size() > 0) {
      // Add detectors
      ui->detectorCombo->addItem("(Path's default)", "");
      ui->detectorSaveCombo->addItem("(Path's default)", "");
      for (auto det : dets) {
        QString name = QString::fromStdString(det);
        ui->detectorCombo->addItem(name, name);
        ui->detectorSaveCombo->addItem(name, name);
      }

      index = ui->detectorCombo->findData(m_properties.detector);
      if (index == -1)
        index = 0;
      BLOCKSIG(ui->detectorCombo, setCurrentIndex(index));

      index = ui->detectorSaveCombo->findData(m_properties.saveDetector);
      if (index == -1)
        index = 0;
      BLOCKSIG(ui->detectorSaveCombo, setCurrentIndex(index));
    }

    for (auto p : m_properties.dofs)
      m_propModel->setDof(p.first, p.second, setEdited);

    // Add optical elements to combo
    for (auto &p : model->opticalElementHierarchy()) {
      auto element = model->resolveElement(p);
      if (element != nullptr) {
        auto optEl = static_cast<RZ::OpticalElement *>(element);
        if (!optEl->opticalSurfaces().empty())
          ui->opticalElementCombo->addItem(
                QIcon(*elementIcon(element)),
                QString::fromStdString(p),
                QVariant::fromValue<RZ::OpticalElement *>(optEl));
      }
    }

    ui->footprintTable->setRowCount(0);
    for (auto &p : m_properties.footprints)
      insertFootprintElement(p);
  }

  refreshUi();
}

void
SimulationPropertiesDialog::parseProperties()
{
  if (m_session == nullptr)
    return;

  switch (ui->tracingType->currentIndex()) {
    case 0:
      m_properties.ttype = TRACER_TYPE_GEOMETRIC_OPTICS;
      break;

    case 1:
      m_properties.ttype = TRACER_TYPE_DIFFRACTION;
      break;
  }

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

  // Artifact generation
  m_properties.saveArtifacts = ui->saveCheck->isChecked();
  m_properties.saveCSV       = ui->saveCSVCheck->isChecked();
  m_properties.clearDetector = ui->clearDetCheck->isChecked();
  m_properties.overwrite     = ui->overwriteResultsCheck->isChecked();

  m_properties.saveDir       = ui->outputDirEdit->text();
  m_properties.saveDetector  = ui->detectorSaveCombo->currentIndex() == -1
                               ? ""
                               : ui->detectorSaveCombo->currentData().toString();
  // Parse expressions
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

void
SimulationPropertiesDialog::parseRepProperties()
{
  m_repProperties.accumulate = ui->accumCheck->isChecked();

  if (ui->fixedColorRadio->isChecked())
    m_repProperties.coloringMode = COLORING_FIXED;
  else if (ui->wlColorRadio->isChecked())
    m_repProperties.coloringMode = COLORING_WAVELENGTH;
  else if (ui->incrementalColorRadio->isChecked())
    m_repProperties.coloringMode = COLORING_CYCLE;

  m_repProperties.fixedBeamColor = m_fixedColorChooser->getColor();
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
      QMessageBox::critical(
            this,
            "Beam properties",
            "Some of the simulation properties contain errors");
    }

    state->setRepresentationProperties(m_repProperties);
  }

  return stateUpdated;
}

void
SimulationPropertiesDialog::accept()
{
  parseProperties();
  parseRepProperties();

  RZGUISingleton::saveSetting(m_repProperties, "Representation");
  RZGUISingleton::sync();

  if (doUpdateState())
    QDialog::accept();
}

void
SimulationPropertiesDialog::sanitizeSaveDirectory()
{
  QFileInfo info(m_properties.saveDir);
  QString   dirName;
  QString   fileName = info.fileName();
  QFileInfo parentInfo(info.dir().path());
  QFileInfo cwdInfo(QDir::currentPath());

  if (!parentInfo.exists() || !parentInfo.isDir() || parentInfo == cwdInfo)
    dirName = ".";
  else
    dirName = parentInfo.path();

  m_properties.saveDir = dirName + "/" + fileName;
}

void
SimulationPropertiesDialog::insertFootprintElement(std::string const &strPath)
{
  if (m_session == nullptr)
    return;

  auto model = m_session->topLevelModel();
  if (model == nullptr)
    return;

  RZ::OpticalElement *element = model->resolveOpticalElement(strPath);
  if (element == nullptr)
    return;

  QPixmap *iconPx = elementIcon(element);

  int row = ui->footprintTable->rowCount();
  ui->footprintTable->insertRow(row);
  ui->footprintTable->setItem(
        row,
        1,
        new QTableWidgetItem(QString::fromStdString(strPath)));

  ui->footprintTable->setItem(
        row,
        2,
        new QTableWidgetItem(QString::fromStdString(element->factory()->name())));

  ui->footprintTable->setItem(
        row,
        3,
        new QTableWidgetItem(QString::number(element->opticalSurfaces().size())));

  ui->footprintTable->item(row, 3)->setTextAlignment(Qt::AlignRight);

  if (iconPx != nullptr) {
    QIcon icon(*iconPx);
    QTableWidgetItem *iconItem = new QTableWidgetItem;
    iconItem->setIcon(icon);
    ui->footprintTable->setItem(row, 0, iconItem);
  }

  ui->footprintTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  ui->footprintTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  ui->footprintTable->horizontalHeader()->resizeSection(0, 24);
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

      sanitizeSaveDirectory(); // To prevent nasty things

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
    QJsonDocument document(m_properties.serialize());
    QString fileName = m_saveSettingsDialog->selectedFiles()[0];
    auto    props    = document.toJson();
    QFile   file(fileName);

    if (file.open(QIODevice::WriteOnly)) {
      file.write(props);
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

void
SimulationPropertiesDialog::onBrowseOutputDir()
{
  QString dir = QFileDialog::getExistingDirectory(
        this,
        "Open output directory",
        m_properties.saveDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (!dir.isNull())
    ui->outputDirEdit->setText(dir);
}

void
SimulationPropertiesDialog::onRepChanged()
{
  refreshUi();
}

void
SimulationPropertiesDialog::onAddFootprint()
{
  if (ui->opticalElementCombo->currentIndex() != -1) {
    QString path = ui->opticalElementCombo->currentText();
    std::string strPath = path.toStdString();

    if (std::find(
          m_properties.footprints.begin(),
          m_properties.footprints.end(),
          strPath) != m_properties.footprints.end())
      return;

    insertFootprintElement(strPath);
    m_properties.footprints.push_back(strPath);
  }
}

void
SimulationPropertiesDialog::onRemoveFootprint()
{
  int row = ui->footprintTable->currentRow();
  if (row != -1) {
    std::string strPath = ui->footprintTable->item(
          row,
          1)->text().toStdString();
    auto where = std::find(
          m_properties.footprints.begin(),
          m_properties.footprints.end(),
          strPath);

    if (where != m_properties.footprints.end()) {
      m_properties.footprints.erase(where);
      ui->footprintTable->removeRow(row);
    }
  }
}

void
SimulationPropertiesDialog::onRemoveAllFootprints()
{
  ui->footprintTable->setRowCount(0);
  m_properties.footprints.clear();
}

QString
SimulationPropertiesDialog::suggestBeamName() const
{
  if (m_properties.beams.empty()) {
    return "New beam";
  } else {
    unsigned num = 0;
    auto &last = m_properties.beams.back();
    std::string stdName = last.name.toStdString();
    const char *cName = stdName.c_str();
    const char *par = strchr(cName, '(');

    if (par != nullptr) {
      if (sscanf(par + 1, "%u", &num) < 1)
        num = 0;
      else
        stdName = stdName.substr(0, static_cast<size_t>(par - cName) - 1);
    }

    return QString::fromStdString(stdName) + " (" + QString::number(num + 1) + ")";
  }
}

void
SimulationPropertiesDialog::onAddBeam()
{
  m_beamPropertiesDialog->setNameHint(suggestBeamName());

  if (m_beamPropertiesDialog->exec()) {
    m_properties.addBeam(m_beamPropertiesDialog->getProperties());
    refreshBeamList();
    refreshUi();
  }
}

void
SimulationPropertiesDialog::onRemoveBeam()
{
  int selectedRow = ui->beamTableWidget->currentRow();

  if (selectedRow >= 0 && selectedRow < m_properties.beamVector.size()) {
    m_properties.removeBeam(selectedRow);
    refreshBeamList();
    refreshUi();
  }
}

void
SimulationPropertiesDialog::onRemoveAllBeams()
{
  m_properties.clearBeams();
  refreshBeamList();
  refreshUi();
}

void
SimulationPropertiesDialog::onEditBeam(int row, int col)
{
  printf("Edit beam: %d, %d\n", row, col);

  if (row >= 0 && row < m_properties.beamVector.size()) {
    m_beamPropertiesDialog->setBeamProperties(*m_properties.beamVector[row]);
    if (m_beamPropertiesDialog->exec()) {
      *m_properties.beamVector[row] = m_beamPropertiesDialog->getProperties();
      refreshBeamList();
    }
  }
}
