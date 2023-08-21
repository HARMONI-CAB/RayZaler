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

class MainWindow : public QMainWindow
{
  Q_OBJECT

  std::list<SimulationSession *> m_sessions;
  QMap<SimulationSession *, SessionTabWidget *> m_sessionToTab;

  void registerSession(SimulationSession *);
  void doOpen();

public:
  void connectAll();

  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
  void onOpen();

private:
  Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
