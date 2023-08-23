#include "SimulationPropertiesDialog.h"
#include "ui_SimulationPropertiesDialog.h"

SimulationPropertiesDialog::SimulationPropertiesDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SimulationPropertiesDialog)
{
  ui->setupUi(this);

  connectAll();
}

SimulationPropertiesDialog::~SimulationPropertiesDialog()
{
  delete ui;
}

void
SimulationPropertiesDialog::connectAll()
{

}

void
SimulationPropertiesDialog::refreshUi()
{
  ui->steps1Label->setEnabled(m_properties.type != SIM_TYPE_ONE_SHOT);
  ui->steps2Label->setEnabled(m_properties.type == SIM_TYPE_2D_SWEEP);

  ui->steps1Spin->setEnabled(m_properties.type != SIM_TYPE_ONE_SHOT);
  ui->steps2Spin->setEnabled(m_properties.type == SIM_TYPE_2D_SWEEP);

  ui->fNumLabel->setEnabled(m_properties.beam  != BEAM_TYPE_COLLIMATED);
  ui->fNumEdit->setEnabled(m_properties.beam   != BEAM_TYPE_COLLIMATED);

  ui->pathCombo->setEnabled(ui->pathCombo->count() > 0);
  ui->detectorCombo->setEnabled(ui->detectorCombo->count() > 0);
}

void
SimulationPropertiesDialog::applyProperties()
{
  ui->pathCombo->clear();
  ui->detectorCombo->clear();

  BLOCKSIG(ui->simTypeCombo,  setCurrentIndex(m_properties.type));
  BLOCKSIG(ui->beamTypeCombo, setCurrentIndex(m_properties.beam));

  BLOCKSIG(ui->diamEdit, setText(m_properties.diameter));
  BLOCKSIG(ui->fNumEdit, setText(m_properties.fNum));
  BLOCKSIG(ui->azimuthEdit, setText(m_properties.azimuth));
  BLOCKSIG(ui->elevationEdit, setText(m_properties.elevation));
  BLOCKSIG(ui->offsetXEdit, setText(m_properties.offsetX));
  BLOCKSIG(ui->offsetYEdit, setText(m_properties.offsetY));

  BLOCKSIG(ui->rayNumberSpin, setValue(m_properties.rays));
  BLOCKSIG(ui->steps1Spin,    setValue(m_properties.Ni));
  BLOCKSIG(ui->steps2Spin,    setValue(m_properties.Nj));

  // Add all paths and detectors

  refreshUi();
}

void
SimulationPropertiesDialog::parseProperties()
{

}

////////////////////////////////////// Slots ///////////////////////////////////
void
SimulationPropertiesDialog::onDataChanged()
{

}

void
SimulationPropertiesDialog::onExprEditChanged()
{

}

void
SimulationPropertiesDialog::onAcceptProperties()
{

}
