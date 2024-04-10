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

class ColorChooserButton;

class SimulationPropertiesDialog : public QDialog
{
  Q_OBJECT

  SimulationSession       *m_session;
  PropertyAndDofExprModel *m_propModel = nullptr;
  SimulationProperties     m_properties;
  RepresentationProperties m_repProperties;

  QFileDialog             *m_openSettingsDialog = nullptr;
  QFileDialog             *m_saveSettingsDialog = nullptr;
  ColorChooserButton      *m_fixedColorChooser = nullptr;

  void connectAll();
  void refreshUi();

  void applySettings();
  void applyProperties(bool setEdited = false);
  void parseProperties();
  void parseRepProperties();
  void sanitizeSaveDirectory();

public:
  explicit SimulationPropertiesDialog(QWidget *parent = nullptr);
  ~SimulationPropertiesDialog() override;

  void setSession(SimulationSession *);

  void setProperties(SimulationProperties const &);

  SimulationProperties properties() const;
  RepresentationProperties repProperties() const;

  bool doLoadFromFile();
  bool doUpdateState();
  void accept() override;

public slots:
  void onExprEditChanged();
  void onBrowseOutputDir();
  void onRepChanged();
  void onDataChanged();
  void onLoadSettings();
  void onExportSettings();

private:
  Ui::SimulationPropertiesDialog *ui;
};

#endif // SIMULATIONPROPERTIESDIALOG_H
