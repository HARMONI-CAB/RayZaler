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

#ifndef EXPORTVIEWDIALOG_H
#define EXPORTVIEWDIALOG_H

#include <QDialog>
#include <Matrix.h>

namespace Ui {
  class ExportViewDialog;
}

class SessionTabWidget;
class RZGUIGLWidget;

class ExportViewDialog : public QDialog
{
  Q_OBJECT

  RZGUIGLWidget     *m_glWidget = nullptr;
  SessionTabWidget  *m_sessionTab = nullptr;
  RZ::Real           m_nominalWidth;
  RZ::Real           m_nominalHeight;

  void               renderAndSave();
  void               connectAll();
  void               adjustHeight(qreal ratio);
  void               adjustWidth(qreal ratio);

public:
  explicit ExportViewDialog(QWidget *parent = nullptr);

  void setSessionTabWidget(SessionTabWidget *);
  ~ExportViewDialog();

private:
  Ui::ExportViewDialog *ui;

public slots:
  void onBrowse();
  void onChangeWidthSpin();
  void onChangeHeightSpin();
  void onLockToggled();
  void onResetSame();
  void onSave();
  void onCancel();
};

#endif // EXPORTVIEWDIALOG_H
