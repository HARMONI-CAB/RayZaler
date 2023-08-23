#ifndef SIMULATIONPROGRESSDIALOG_H
#define SIMULATIONPROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
  class SimulationProgressDialog;
}

class SimulationProgressDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SimulationProgressDialog(QWidget *parent = nullptr);
  ~SimulationProgressDialog();

private:
  Ui::SimulationProgressDialog *ui;
};

#endif // SIMULATIONPROGRESSDIALOG_H
