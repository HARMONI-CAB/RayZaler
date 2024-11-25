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

#ifndef BEAMPROPERTIESDIALOG_H
#define BEAMPROPERTIESDIALOG_H

#include <QDialog>
#include "SimulationProperties.h"

namespace Ui {
  class BeamPropertiesDialog;
}

class ColorChooserButton;
class SimulationSession;
class QFileDialog;

class BeamPropertiesDialog : public QDialog
{
  Q_OBJECT

  SimulationBeamProperties m_properties;
  SimulationSession       *m_session         = nullptr;
  ColorChooserButton      *m_colorChooser    = nullptr;
  QFileDialog             *m_openImageDialog = nullptr;

  void connectAll();
  void parseProperties();
  void refreshUi();
  void refreshUiState();

public:
  explicit BeamPropertiesDialog(SimulationSession *, QWidget *parent = nullptr);
  virtual ~BeamPropertiesDialog() override;

  void setNameHint(QString const &);
  void setColorHint(QColor const &);
  void setSession(SimulationSession *);

  void setBeamProperties(SimulationBeamProperties const &);
  SimulationBeamProperties getProperties();
  void highlightFaultyField(QString const &);

public slots:
  void onBrowse();
  void onEditDirectionCosines();
  void onExprEditChanged();
  void onDataChanged();

private:
  Ui::BeamPropertiesDialog *ui;
};

#endif // BEAMPROPERTIESDIALOG_H
