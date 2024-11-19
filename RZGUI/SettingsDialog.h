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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ColorSettings.h"

namespace Ui {
  class SettingsDialog;
}

class ColorChooserButton;
class CustomTabStyle;

class SettingsDialog : public QDialog
{
  Q_OBJECT

  ColorChooserButton *m_bgAboveColor = nullptr;
  ColorChooserButton *m_bgBelowColor = nullptr;
  ColorChooserButton *m_gridColor    = nullptr;
  ColorChooserButton *m_pathColor    = nullptr;
  CustomTabStyle     *m_style        = nullptr;

  mutable ColorSettings m_colorSettings;
  void                  loadColorSettings();
  void                  storeColorSettings() const;

public:
  explicit SettingsDialog(QWidget *parent = nullptr);
  virtual ~SettingsDialog() override;

  ColorSettings const &colorSettings() const;

private:
  Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
