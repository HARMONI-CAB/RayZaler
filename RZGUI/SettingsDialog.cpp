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

#include "SettingsDialog.h"
#include "ColorChooserButton.h"
#include "ui_SettingsDialog.h"
#include "RZGUI.h"
#include "GUIHelpers.h"
#include <QProxyStyle>
#include <QTabBar>
#include <QStyleOptionTab>

class CustomTabStyle : public QProxyStyle {
public:
  QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                         const QSize& size, const QWidget* widget) const {
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == QStyle::CT_TabBarTab) {
      s.transpose();
    }
    return s;
  }

  void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
    if (element == CE_TabBarTabLabel) {
      if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
        QStyleOptionTab opt(*tab);
        opt.shape = QTabBar::RoundedNorth;
        QProxyStyle::drawControl(element, &opt, painter, widget);
        return;
      }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
  }
};

SettingsDialog::SettingsDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SettingsDialog)
{
  ui->setupUi(this);

  m_bgAboveColor = new ColorChooserButton(ui->tab);
  ui->gridLayout_2->addWidget(m_bgAboveColor, 0, 1, 1, 1);

  m_bgBelowColor = new ColorChooserButton(ui->tab);
  ui->gridLayout_2->addWidget(m_bgBelowColor, 1, 1, 1, 1);

  m_gridColor    = new ColorChooserButton(ui->tab);
  ui->gridLayout_2->addWidget(m_gridColor, 0, 3, 1, 1);

  m_pathColor    = new ColorChooserButton(ui->tab);
  ui->gridLayout_2->addWidget(m_pathColor, 1, 3, 1, 1);

  ui->tabWidget->tabBar()->setStyle(new CustomTabStyle);
  loadColorSettings();
}

SettingsDialog::~SettingsDialog()
{
  delete ui;
}

void
SettingsDialog::loadColorSettings()
{
  RZGUISingleton::loadColorSettings(m_colorSettings);

  BLOCKSIG(m_bgAboveColor, setColor(m_colorSettings.bgAbove));
  BLOCKSIG(m_bgBelowColor, setColor(m_colorSettings.bgBelow));
  BLOCKSIG(m_gridColor,    setColor(m_colorSettings.grid));
  BLOCKSIG(m_pathColor,    setColor(m_colorSettings.path));

}

void
SettingsDialog::storeColorSettings() const
{
  m_colorSettings.bgAbove = m_bgAboveColor->getColor();
  m_colorSettings.bgBelow = m_bgBelowColor->getColor();
  m_colorSettings.grid = m_gridColor->getColor();
  m_colorSettings.path = m_pathColor->getColor();

  RZGUISingleton::saveColorSettings(m_colorSettings);
}

ColorSettings const &
SettingsDialog::colorSettings() const
{
  storeColorSettings();

  return m_colorSettings;
}
