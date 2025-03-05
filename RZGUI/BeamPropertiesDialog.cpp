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

#include "BeamPropertiesDialog.h"
#include "ui_BeamPropertiesDialog.h"
#include "ColorChooserButton.h"
#include "SimulationSession.h"
#include <QFileDialog>
#include <QFontMetrics>

void
BeamPropertiesDialog::connectAll()
{
  connect(
        ui->wavelengthColorButton,
        SIGNAL(toggled(bool)),
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
        ui->angleSpanEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onExprEditChanged()));

  connect(
        ui->uXEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onExprEditChanged()));

  connect(
        ui->uYEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onExprEditChanged()));

  connect(
        ui->uXEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onEditDirectionCosines()));

  connect(
        ui->uYEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onEditDirectionCosines()));

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
        ui->offsetZEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onExprEditChanged()));

  connect(
        ui->wlEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(onExprEditChanged()));

  connect(
        ui->beamTypeCombo,
        SIGNAL(activated(int)),
        this,
        SLOT(onDataChanged()));

  connect(
        ui->objectShapeCombo,
        SIGNAL(activated(int)),
        this,
        SLOT(onDataChanged()));

  connect(
        ui->beamShapeCombo,
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
        ui->browseButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onBrowse()));
}

void
BeamPropertiesDialog::parseProperties()
{
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

  switch (ui->beamShapeCombo->currentIndex()) {
    case 0:
      m_properties.shape = RZ::Circular;
      break;

    case 1:
      m_properties.shape = RZ::Ring;
      break;

    case 2:
      m_properties.shape = RZ::Point;
      break;
  }

  switch (ui->objectShapeCombo->currentIndex()) {
    case 0:
      m_properties.objectShape = RZ::PointLike;
      break;

    case 1:
      m_properties.objectShape = RZ::CircleLike;
      break;

    case 2:
      m_properties.objectShape = RZ::RingLike;
      break;

    case 3:
      m_properties.objectShape = RZ::Extended;
      break;
  }

  m_properties.name         = ui->beamNameEdit->text();
  m_properties.color        = m_colorChooser->getColor();
  m_properties.diameter     = ui->diamEdit->text();
  m_properties.span         = ui->angleSpanEdit->text();
  m_properties.fNum         = ui->fNumEdit->text();
  m_properties.uX           = ui->uXEdit->text();
  m_properties.uY           = ui->uYEdit->text();
  m_properties.offsetX      = ui->offsetXEdit->text();
  m_properties.offsetY      = ui->offsetYEdit->text();
  m_properties.offsetZ      = ui->offsetZEdit->text();
  m_properties.wavelength   = ui->wlEdit->text();
  m_properties.random       = ui->beamSamplingCombo->currentIndex() == 1;
  m_properties.colorByWl    = ui->wavelengthColorButton->isChecked();
  m_properties.focalPlane   = ui->focalPlaneCombo->currentData().toString();
  m_properties.apertureStop = ui->apertureCombo->currentData().toString();
  m_properties.path         = ui->pathEdit->text();
  m_properties.rays         = ui->rayNumberSpin->value();

}

void
BeamPropertiesDialog::refreshUi()
{
  int originIndex = 0;
  int objShapeIndex = 0;

  BLOCKSIG(ui->beamTypeCombo,     setCurrentIndex(m_properties.beam));
  BLOCKSIG(ui->beamShapeCombo,    setCurrentIndex(m_properties.shape));

  BLOCKSIG(m_colorChooser,        setColor(m_properties.color));
  BLOCKSIG(ui->wavelengthColorButton, setChecked(m_properties.colorByWl));
  BLOCKSIG(ui->beamNameEdit,      setText(m_properties.name));
  BLOCKSIG(ui->angleSpanEdit,     setText(m_properties.span));
  BLOCKSIG(ui->diamEdit,          setText(m_properties.diameter));
  BLOCKSIG(ui->fNumEdit,          setText(m_properties.fNum));
  BLOCKSIG(ui->uXEdit,            setText(m_properties.uX));
  BLOCKSIG(ui->uYEdit,            setText(m_properties.uY));
  BLOCKSIG(ui->offsetXEdit,       setText(m_properties.offsetX));
  BLOCKSIG(ui->offsetYEdit,       setText(m_properties.offsetY));
  BLOCKSIG(ui->offsetZEdit,       setText(m_properties.offsetZ));
  BLOCKSIG(ui->wlEdit,            setText(m_properties.wavelength));
  BLOCKSIG(ui->beamSamplingCombo, setCurrentIndex(m_properties.random ? 1 : 0));
  BLOCKSIG(ui->pathEdit,          setText(m_properties.path));
  BLOCKSIG(ui->rayNumberSpin,     setValue(m_properties.rays));

  switch (m_properties.ref) {
    case BEAM_REFERENCE_INPUT_ELEMENT:
      originIndex = 0;
      break;

    case BEAM_REFERENCE_APERTURE_STOP:
      originIndex = 1;
      break;

    case BEAM_REFERENCE_FOCAL_PLANE:
      originIndex = 2;
      break;
  }

  BLOCKSIG(ui->originCombo, setCurrentIndex(originIndex));


  switch (m_properties.objectShape) {
    case RZ::PointLike:
      objShapeIndex = 0;
      break;

    case RZ::CircleLike:
      objShapeIndex = 1;
      break;

    case RZ::RingLike:
      objShapeIndex = 2;
      break;

    case RZ::Extended:
      objShapeIndex = 3;
      break;
  }

  BLOCKSIG(ui->objectShapeCombo, setCurrentIndex(objShapeIndex));
}

void
BeamPropertiesDialog::setNameHint(QString const &name)
{
  m_properties.name = name;
  refreshUi();
}

void
BeamPropertiesDialog::setColorHint(QColor const &color)
{
  m_properties.color = color;
  refreshUi();
}

void
BeamPropertiesDialog::refreshUiState()
{
  bool haveApertures   = ui->apertureCombo->count() > 0;
  bool haveFocalPlanes = ui->focalPlaneCombo->count() > 0;
  bool fileEnabled     = m_properties.objectShape == RZ::Extended;

  ui->pathLabel->setEnabled(fileEnabled);
  ui->pathEdit->setEnabled(fileEnabled);
  ui->browseButton->setEnabled(fileEnabled);

  m_colorChooser->setEnabled(!ui->wavelengthColorButton->isChecked());

  switch (ui->originCombo->currentIndex()) {
    case 0:
      ui->refPlaneStack->setCurrentWidget(ui->inputElementPage);
      ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
      break;

    case 1:
      ui->refPlaneStack->setCurrentWidget(ui->aperturePage);
      ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(haveApertures);
      break;

    case 2:
      ui->refPlaneStack->setCurrentWidget(ui->focalPlanePage);
      ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(haveFocalPlanes);
      break;
  }

  ui->fNumLabel->setEnabled(m_properties.beam  != BEAM_TYPE_COLLIMATED);
  ui->fNumEdit->setEnabled(m_properties.beam   != BEAM_TYPE_COLLIMATED);

  ui->diamEdit->setEnabled(m_properties.shape != RZ::Point);

  ui->apertureCombo->setEnabled(haveApertures);
  ui->focalPlaneCombo->setEnabled(haveFocalPlanes);

  onEditDirectionCosines();
}

void
BeamPropertiesDialog::setSession(SimulationSession *session)
{
  m_session = session;

  ui->apertureCombo->clear();
  ui->focalPlaneCombo->clear();

  if (m_session != nullptr) {
    int index;
    auto model = m_session->topLevelModel();
    auto stops = model->apertureStops();
    auto fps   = model->focalPlanes();

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
}

void
BeamPropertiesDialog::setBeamProperties(SimulationBeamProperties const &beam)
{
  m_properties = beam;

  refreshUi();
  refreshUiState();
}

SimulationBeamProperties
BeamPropertiesDialog::getProperties()
{
  parseProperties();
  return m_properties;
}

void
BeamPropertiesDialog::highlightFaultyField(QString const &failed)
{
  QLineEdit *edit = nullptr;

  if (failed.size() != 0) {
    if (failed == "diameter")
      edit = ui->diamEdit;
    else if (failed == "fNum")
      edit = ui->fNumEdit;
    else if (failed == "uX")
      edit = ui->uXEdit;
    else if (failed == "uY")
      edit = ui->uYEdit;
    else if (failed == "offsetX")
      edit = ui->offsetXEdit;
    else if (failed == "offsetY")
      edit = ui->offsetYEdit;
    else if (failed == "offsetZ")
      edit = ui->offsetZEdit;
    else if (failed == "wavelength")
      edit = ui->wlEdit;
  }

  edit->setStyleSheet("background-color: #ffbfbf; color: black");
}

BeamPropertiesDialog::BeamPropertiesDialog(
    SimulationSession *sess,
    QWidget *parent) : QDialog(parent), ui(new Ui::BeamPropertiesDialog)
{
  ui->setupUi(this);

  auto minAzElWidth = ui->azLabel->fontMetrics().horizontalAdvance("+00.00 º");

  ui->azLabel->setMinimumWidth(minAzElWidth);
  ui->elLabel->setMinimumWidth(minAzElWidth);

  m_openImageDialog = new QFileDialog(this);

  m_openImageDialog->setWindowTitle("Define radiance map");
  m_openImageDialog->setFileMode(QFileDialog::ExistingFile);
  m_openImageDialog->setAcceptMode(QFileDialog::AcceptOpen);
  m_openImageDialog->setNameFilter("PNG image (*.png);;All files (*)");

  m_colorChooser = new ColorChooserButton(this);
  ui->gridLayout_9->addWidget(m_colorChooser, 0, 3, 1, 1);

  connectAll();
  refreshUi();
  refreshUiState();

  setSession(sess);

  setColorHint(QColor::fromRgb(255, 255, 0));
}

BeamPropertiesDialog::~BeamPropertiesDialog()
{
  delete ui;
}

/////////////////////////////////// Slots //////////////////////////////////////
void
BeamPropertiesDialog::onBrowse()
{
  if (m_openImageDialog->exec()
      && !m_openImageDialog->selectedFiles().empty()) {
    QString fileName = m_openImageDialog->selectedFiles()[0];

    m_properties.path = fileName;
    refreshUi();
  }
}

void
BeamPropertiesDialog::onEditDirectionCosines()
{
  bool uXok, uYok;
  qreal uX, uY;

  uX = ui->uXEdit->text().toDouble(&uXok);
  uY = ui->uYEdit->text().toDouble(&uYok);

  if (uXok && uYok) {
    qreal uZ = 1 - uX * uX - uY * uY;

    if (uZ < 0) {
      highlightFaultyField("uX");
      highlightFaultyField("uY");

      ui->azLabel->setText("N/A");
      ui->elLabel->setText("N/A");
      ui->uZLabel->setText("Invalid");
    } else {
      uZ = -sqrt(uZ);

      qreal az = atan2(uX, uY); // Yes, this is right.
      qreal el = asin(-uZ);

      ui->azLabel->setText(QString::asprintf("%+.4gº", RZ::rad2deg(az)));
      ui->elLabel->setText(QString::asprintf("%+.4gº", RZ::rad2deg(el)));
      ui->uZLabel->setText(QString::number(uZ));
    }
  } else {
    ui->azLabel->setText("N/A");
    ui->elLabel->setText("N/A");
    ui->uZLabel->setText("N/A");
  }
}

void
BeamPropertiesDialog::onExprEditChanged()
{
  auto sender = static_cast<QLineEdit *>(QObject::sender());

  sender->setStyleSheet("");
}

void
BeamPropertiesDialog::onDataChanged()
{
  parseProperties();
  refreshUiState();
}

