#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "SimulationSession.h"
#include "SessionTabWidget.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  connectAll();
}

void
MainWindow::connectAll()
{
  connect(
        ui->actionOpen,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onOpen()));
}

void
MainWindow::registerSession(SimulationSession *session)
{
  SessionTabWidget *widget = new SessionTabWidget(session);

  ui->sessionTabWidget->addTab(widget, session->fileName());
  ui->sessionTabWidget->setCurrentWidget(widget);

  m_sessions.push_back(session);
  m_sessionToTab.insert(session, widget);
}

void
MainWindow::doOpen()
{
  QFileDialog dialog(this);
  QStringList filters;
  bool done = false;


  filters <<
             "RayZaler model files (*.rzm)" <<
             "All files (*)";

  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setNameFilters(filters);

  do {
    if (dialog.exec()) {
      auto files = dialog.selectedFiles();
      if (files.size() > 0) {
        auto firstFile = files[0];
        try {
          registerSession(new SimulationSession(firstFile, this));
          done = true;
        } catch (std::runtime_error &e) {
          QMessageBox::critical(
                this,
                "Load model file",
                QString::fromStdString(e.what()));
          done = false;
        }
      } else {
        done = true;
      }
    } else {
      done = true;
    }
  } while (!done);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void
MainWindow::onOpen()
{
  doOpen();
}
