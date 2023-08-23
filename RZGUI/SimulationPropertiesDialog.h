#ifndef SIMULATIONPROPERTIESDIALOG_H
#define SIMULATIONPROPERTIESDIALOG_H

#include <QDialog>
#include "SimulationSession.h"

namespace Ui {
  class SimulationPropertiesDialog;
}

class SimulationPropertiesDialog : public QDialog
{
  Q_OBJECT

  SimulationSession *m_session;
  SimulationProperties m_properties;

  void connectAll();
  void refreshUi();
  void applyProperties();
  void parseProperties();

public:
  explicit SimulationPropertiesDialog(QWidget *parent = nullptr);
  ~SimulationPropertiesDialog() override;

  void setSession(SimulationSession *);

  void setProperties(SimulationProperties const &);
  SimulationProperties properties() const;

  void accept() override;

public slots:
  void onExprEditChanged();
  void onDataChanged();

private:
  Ui::SimulationPropertiesDialog *ui;
};

#endif // SIMULATIONPROPERTIESDIALOG_H
