#ifndef DETECTORDIALOG_H
#define DETECTORDIALOG_H

#include <QDialog>

namespace Ui {
  class DetectorDialog;
}

class DetectorDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DetectorDialog(QWidget *parent = nullptr);
  ~DetectorDialog();

private:
  Ui::DetectorDialog *ui;
};

#endif // DETECTORDIALOG_H
