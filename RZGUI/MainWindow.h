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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <list>
#include <QMap>
#include <Logger.h>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SimulationSession;
class SessionTabWidget;
class PropertyAndDofTableModel;
class ElementPropertyModel;
class OMTreeModel;
class SimulationPropertiesDialog;
class QFileDialog;
class DOFWidget;
class AboutDialog;
class ExportViewDialog;
class SettingsDialog;

class ColorSettings;

struct SessionUI {
  SessionTabWidget *tab       = nullptr;
  DOFWidget        *dofWidget = nullptr;
};

struct DelayedFile {
  QString path;
  QString simConfigPath;
};

class MainWindow : public QMainWindow, public RZ::Logger
{
  Q_OBJECT

  SimulationSession *m_currSession = nullptr;

  std::list<SimulationSession *> m_sessions;
  std::list<DelayedFile>         m_delayedFiles;
  QMap<SimulationSession *, SessionUI> m_sessionToUi;

  QString                     m_lastOpenDir;
  SimulationPropertiesDialog *m_simPropertiesDialog = nullptr;
  PropertyAndDofTableModel   *m_propModel           = nullptr;
  ElementPropertyModel       *m_compPropModel       = nullptr;
  OMTreeModel                *m_omModel             = nullptr;
  QModelIndex                 m_currentIndex;
  AboutDialog                *m_aboutDialog         = nullptr;
  ExportViewDialog           *m_exportViewDialog    = nullptr;
  SettingsDialog             *m_settingsDialog      = nullptr;

  void refreshCurrentSession();
  void refreshCurrentElement();
  void registerSession(SimulationSession *);
  void doOpen();
  void doReload();
  void reconnectModels();

  void applyColorSettings(ColorSettings const &);

  void connectAll();

  void initDOFWidget(SessionUI &sessUI, SimulationSession *sess);
  void finalizeDOFWidget(SessionUI &sessUI);
  void openDelayedFiles();
  void updateFootprintMenu(SimulationSession *session);

  SessionTabWidget *currentSessionWidget() const;

public:
  virtual void closeEvent(QCloseEvent *event) override;
  virtual void logFunction(
          RZ::LogLevel level,
          std::string const &file,
          int line,
          std::string const &message) override;

  MainWindow(QWidget *parent = nullptr);
  SimulationSession *openModelFile(QString file);
  void pushDelayedOpenFile(const char *path, const char *simCfg = nullptr);
  ~MainWindow() override;
  void notifyReady();
  void keyPressEvent(QKeyEvent *event) override;

signals:
  void ready();
  
public slots:
  void onShow();

  void onOpen();
  void onReload();
  void onCloseTab(int);
  void onTabChanged();

  void onAnimStart();
  void onAnimEnd();
  void onAnimPause();
  void onAnimPlay();
  void onAnimStop();

  void onSimulationLoadAndRun();
  void onSimulationEditProperties();
  void onSimulationRun();
  void onClearBeam();
  void onSimulationShowResult();

  void onModelsChanged();
  void onDofChanged(
      const QModelIndex &topLeft,
      const QModelIndex &bottomRight,
      const QList<int> &roles = QList<int>());

  void onTreeItemSelectionChanged();
  void onChangeView();
  void onChangeDisplay();
  void onToggleCurrent();
  void onUpdateModel();
  void onModelSource();
  void onCenterToSelected();
  void onExportImage();
  void onOpenPreferences();

  void onSimulationResults();

private:
  Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
