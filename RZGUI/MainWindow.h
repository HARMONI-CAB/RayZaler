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
class OMTreeModel;
class SimulationPropertiesDialog;
class QFileDialog;

class MainWindow : public QMainWindow, public RZ::Logger
{
  Q_OBJECT

  SimulationSession *m_currSession = nullptr;

  std::list<SimulationSession *> m_sessions;
  QMap<SimulationSession *, SessionTabWidget *> m_sessionToTab;

  QString                     m_lastOpenDir;
  SimulationPropertiesDialog *m_simPropertiesDialog = nullptr;
  PropertyAndDofTableModel   *m_propModel = nullptr;
  OMTreeModel                *m_omModel   = nullptr;

  void refreshCurrentSession();
  void registerSession(SimulationSession *);
  void doOpen();

  void reconnectModels();
  void connectAll();

public:
  virtual void logFunction(
          RZ::LogLevel level,
          std::string const &file,
          int line,
          std::string const &message) override;

  MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

  void keyPressEvent(QKeyEvent *event) override;

public slots:
  void onOpen();
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
  void onSimulationShowResult();

  void onModelsChanged();
  void onDofChanged(
      const QModelIndex &topLeft,
      const QModelIndex &bottomRight,
      const QList<int> &roles = QList<int>());

  void onTreeItemSelectionChanged();
  void onChangeView();
  void onChangeDisplay();

private:
  Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
