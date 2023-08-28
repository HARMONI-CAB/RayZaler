#include "DetectorDialog.h"
#include "ui_DetectorDialog.h"

DetectorDialog::DetectorDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DetectorDialog)
{
  ui->setupUi(this);
}

DetectorDialog::~DetectorDialog()
{
  delete ui;
}
