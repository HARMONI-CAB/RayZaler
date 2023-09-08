#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <list>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SimulationSession;
class SessionTabWidget;
class PropertyAndDofTableModel;
class OMTreeModel;
class SimulationPropertiesDialog;

class MainWindow : public QMainWindow
{
  Q_OBJECT

  SimulationSession *m_currSession = nullptr;

  std::list<SimulationSession *> m_sessions;
  QMap<SimulationSession *, SessionTabWidget *> m_sessionToTab;

  SimulationPropertiesDialog *m_simPropertiesDialog = nullptr;
  PropertyAndDofTableModel *m_propModel = nullptr;
  OMTreeModel              *m_omModel   = nullptr;

  void refreshCurrentSession();
  void registerSession(SimulationSession *);
  void doOpen();

  void reconnectModels();
  void connectAll();
public:

  MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

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
