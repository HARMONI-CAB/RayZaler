#include "BeamPropertiesDialog.h"
#include "ui_BeamPropertiesDialog.h"
#include "ColorChooserButton.h"
#include "SimulationSession.h"

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
  }

  m_properties.name         = ui->beamNameEdit->text();
  m_properties.color        = m_colorChooser->getColor();
  m_properties.diameter     = ui->diamEdit->text();
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

  m_properties.rays         = ui->rayNumberSpin->value();

}

void
BeamPropertiesDialog::refreshUi()
{
  int originIndex = 0;

  BLOCKSIG(ui->beamTypeCombo,     setCurrentIndex(m_properties.beam));
  BLOCKSIG(ui->beamShapeCombo,    setCurrentIndex(m_properties.shape));

  BLOCKSIG(m_colorChooser,        setColor(m_properties.color));
  BLOCKSIG(ui->wavelengthColorButton, setChecked(m_properties.colorByWl));
  BLOCKSIG(ui->beamNameEdit,      setText(m_properties.name));
  BLOCKSIG(ui->diamEdit,          setText(m_properties.diameter));
  BLOCKSIG(ui->fNumEdit,          setText(m_properties.fNum));
  BLOCKSIG(ui->uXEdit,            setText(m_properties.uX));
  BLOCKSIG(ui->uYEdit,            setText(m_properties.uY));
  BLOCKSIG(ui->offsetXEdit,       setText(m_properties.offsetX));
  BLOCKSIG(ui->offsetYEdit,       setText(m_properties.offsetY));
  BLOCKSIG(ui->offsetZEdit,       setText(m_properties.offsetZ));
  BLOCKSIG(ui->wlEdit,            setText(m_properties.wavelength));
  BLOCKSIG(ui->beamSamplingCombo, setCurrentIndex(m_properties.random ? 1 : 0));

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

  ui->apertureCombo->setEnabled(haveApertures);
  ui->focalPlaneCombo->setEnabled(haveFocalPlanes);
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

}

SimulationBeamProperties
BeamPropertiesDialog::getProperties() const
{
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

  m_colorChooser = new ColorChooserButton(this);
  ui->gridLayout_9->addWidget(m_colorChooser, 0, 4, 1, 1);

  connectAll();
  refreshUi();
  refreshUiState();

  setSession(sess);
}

BeamPropertiesDialog::~BeamPropertiesDialog()
{
  delete ui;
}

/////////////////////////////////// Slots //////////////////////////////////////
void
BeamPropertiesDialog::onBrowse()
{
  printf("TODO: Implement me!\n");
}

void
BeamPropertiesDialog::onEditDirectionCosines()
{
  printf("TODO: Implement me!\n");
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

