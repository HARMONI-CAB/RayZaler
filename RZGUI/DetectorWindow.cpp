#include "DetectorWindow.h"
#include "ui_DetectorWindow.h"
#include "ImageNavWidget.h"
#include "SimulationSession.h"

DetectorWindow::DetectorWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::DetectorWindow)
{
  ui->setupUi(this);

  m_navWidget = new ImageNavWidget();
  m_navWidget->setAutoScale(true);

  setCentralWidget(m_navWidget);
}

DetectorWindow::~DetectorWindow()
{
  delete ui;
}

void
DetectorWindow::setSession(SimulationSession *session)
{
  RZ::Detector *detector = nullptr;
  m_session = session;

  if (m_session != nullptr) {
    auto model = m_session->topLevelModel();
    auto detectors = model->detectors();

    if (detectors.size() > 0)
      detector = model->lookupDetector(detectors.front());
  }

  m_navWidget->setDetector(detector);
}

void
DetectorWindow::refreshImage()
{
  m_navWidget->recalcImage();
  m_navWidget->update();
}

void
DetectorWindow::closeEvent(QCloseEvent *)
{
  hide();
}
