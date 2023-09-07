#ifndef SIMULATIONPROPERTIESDIALOG_H
#define SIMULATIONPROPERTIESDIALOG_H

#include <QDialog>
#include "SimulationSession.h"

namespace Ui {
  class SimulationPropertiesDialog;
}

class PropertyAndDofExprModel;
class QFileDialog;

#define MAX_SIMULATION_CONFIG_FILE_SIZE (1 << 20)

class SimulationPropertiesDialog : public QDialog
{
  Q_OBJECT

  SimulationSession       *m_session;
  PropertyAndDofExprModel *m_propModel = nullptr;
  SimulationProperties     m_properties;
  QFileDialog             *m_openSettingsDialog = nullptr;
  QFileDialog             *m_saveSettingsDialog = nullptr;

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
  void onLoadSettings();
  void onExportSettings();

private:
  Ui::SimulationPropertiesDialog *ui;
};

#endif // SIMULATIONPROPERTIESDIALOG_H
