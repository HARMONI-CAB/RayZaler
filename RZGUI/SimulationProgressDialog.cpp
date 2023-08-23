#include "SimulationProgressDialog.h"
#include "ui_SimulationProgressDialog.h"

SimulationProgressDialog::SimulationProgressDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SimulationProgressDialog)
{
  ui->setupUi(this);
}

SimulationProgressDialog::~SimulationProgressDialog()
{
  delete ui;
}
