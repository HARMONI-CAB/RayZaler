#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <list>
#include <QMap>
#include <Logger.h>

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

struct SessionUI {
  SessionTabWidget *tab       = nullptr;
  DOFWidget        *dofWidget = nullptr;
};

class MainWindow : public QMainWindow, public RZ::Logger
{
  Q_OBJECT

  SimulationSession *m_currSession = nullptr;

  std::list<SimulationSession *> m_sessions;
  std::list<QString>             m_delayedFiles;
  QMap<SimulationSession *, SessionUI> m_sessionToUi;

  QString                     m_lastOpenDir;
  SimulationPropertiesDialog *m_simPropertiesDialog = nullptr;
  PropertyAndDofTableModel   *m_propModel           = nullptr;
  ElementPropertyModel       *m_compPropModel       = nullptr;
  OMTreeModel                *m_omModel             = nullptr;
  AboutDialog                *m_aboutDialog         = nullptr;
  ExportViewDialog           *m_exportViewDialog    = nullptr;

  void refreshCurrentSession();
  void refreshCurrentElement();
  void registerSession(SimulationSession *);
  void doOpen();
  void doReload();

  void reconnectModels();
  void connectAll();

  void initDOFWidget(SessionUI &sessUI, SimulationSession *sess);
  void finalizeDOFWidget(SessionUI &sessUI);
  void openDelayedFiles();

public:
  virtual void logFunction(
          RZ::LogLevel level,
          std::string const &file,
          int line,
          std::string const &message) override;

  MainWindow(QWidget *parent = nullptr);
  bool openModelFile(QString file);
  void pushDelayedOpenFile(QString file);
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

private:
  Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
