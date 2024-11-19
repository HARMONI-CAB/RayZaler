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

#ifndef SIMULATIONPROPERTIESDIALOG_H
#define SIMULATIONPROPERTIESDIALOG_H

#include <QDialog>
#include "SimulationSession.h"

namespace Ui {
  class SimulationPropertiesDialog;
}

class PropertyAndDofExprModel;
class QFileDialog;
class BeamPropertiesDialog;

#define MAX_SIMULATION_CONFIG_FILE_SIZE (1 << 20)

class ColorChooserButton;

class SimulationPropertiesDialog : public QDialog
{
  Q_OBJECT

  SimulationSession       *m_session;
  PropertyAndDofExprModel *m_propModel = nullptr;
  SimulationProperties     m_properties;

  BeamPropertiesDialog    *m_beamPropertiesDialog = nullptr;
  QFileDialog             *m_openSettingsDialog = nullptr;
  QFileDialog             *m_saveSettingsDialog = nullptr;

  void connectAll();

  void applyProperties(bool setEdited = false);
  void parseProperties();
  void sanitizeSaveDirectory();
  void insertFootprintElement(std::string const &);
  void refreshBeamList();

  QString suggestBeamName() const;

public:
  explicit SimulationPropertiesDialog(QWidget *parent = nullptr);
  ~SimulationPropertiesDialog() override;

  void setSession(SimulationSession *);

  void setProperties(SimulationProperties const &);

  SimulationProperties properties() const;

  bool doLoadFromFile();
  bool doUpdateState();
  void accept() override;

public slots:
  void refreshUi();
  void onBrowseOutputDir();
  void onDataChanged();
  void onLoadSettings();
  void onExportSettings();

  void onAddFootprint();
  void onRemoveFootprint();
  void onRemoveAllFootprints();

  void onAddBeam();
  void onRemoveBeam();
  void onRemoveAllBeams();
  void onEditBeam(int, int);

private:
  Ui::SimulationPropertiesDialog *ui;
};

#endif // SIMULATIONPROPERTIESDIALOG_H
