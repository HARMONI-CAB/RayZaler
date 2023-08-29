#include "DetectorWindow.h"
#include "ui_DetectorWindow.h"
#include "ImageNavWidget.h"
#include "SimulationSession.h"
#include "GUIHelpers.h"

DetectorWindow::DetectorWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::DetectorWindow)
{
  ui->setupUi(this);

  m_navWidget = new ImageNavWidget();
  m_navWidget->setAutoScale(true);

  ui->viewGrid->addWidget(m_navWidget, 0, 0);

  connectAll();
}

DetectorWindow::~DetectorWindow()
{
  delete ui;
}

void
DetectorWindow::refreshUi()
{
  if (m_detector != nullptr) {
    qreal zoom    = m_navWidget->zoom();
    auto imgSize  = QSizeF(m_navWidget->imageSize()) * zoom;
    auto viewSize = QSizeF(ui->viewFrame->size());
    auto pos      = m_navWidget->currPoint() * zoom;

    BLOCKSIG_BEGIN(ui->horizontalScrollBar);
    if (viewSize.width() < imgSize.width()) {
      auto horizontalRange = static_cast<int>(
            imgSize.width() - viewSize.width());

      ui->horizontalScrollBar->setMinimum(-horizontalRange / 2);
      ui->horizontalScrollBar->setMaximum(
            horizontalRange - horizontalRange / 2);


      ui->horizontalScrollBar->setPageStep(static_cast<int>(viewSize.width()));
      ui->horizontalScrollBar->setValue(static_cast<int>(pos.x()));
      ui->horizontalScrollBar->setEnabled(true);
    } else {
      ui->horizontalScrollBar->setRange(0, 0);
      ui->horizontalScrollBar->setEnabled(false);
    }
    BLOCKSIG_END();

    BLOCKSIG_BEGIN(ui->verticalScrollBar);
    if (viewSize.height() < imgSize.height()) {
      auto verticalRange = static_cast<int>(
            imgSize.height() - viewSize.height());
      ui->verticalScrollBar->setMinimum(-verticalRange / 2);
      ui->verticalScrollBar->setMaximum(verticalRange - verticalRange / 2);

      ui->verticalScrollBar->setPageStep(static_cast<int>(viewSize.height()));
      ui->verticalScrollBar->setValue(static_cast<int>(pos.y()));
      ui->verticalScrollBar->setEnabled(true);
    } else {
      ui->verticalScrollBar->setRange(0, 0);
      ui->verticalScrollBar->setEnabled(false);
    }
    BLOCKSIG_END();
  }
}

void
DetectorWindow::resizeEvent(QResizeEvent *)
{
  refreshUi();
}

void
DetectorWindow::connectAll()
{
  connect(
        m_navWidget,
        SIGNAL(viewChanged()),
        this,
        SLOT(onViewChanged()));

  connect(
        ui->actionClear,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onClearDetector()));

  connect(
        ui->horizontalScrollBar,
        SIGNAL(valueChanged(int)),
        this,
        SLOT(onScrollBarsChanged()));

  connect(
        ui->verticalScrollBar,
        SIGNAL(valueChanged(int)),
        this,
        SLOT(onScrollBarsChanged()));
}

void
DetectorWindow::setSession(SimulationSession *session)
{
  QString title;
  RZ::Detector *detector = nullptr;
  m_session = session;

  if (m_session != nullptr) {
    auto model = m_session->topLevelModel();
    auto detectors = model->detectors();

    if (detectors.size() > 0)
      detector = model->lookupDetector(detectors.front());
  }

  m_detector = detector;
  m_navWidget->setDetector(detector);

  if (detector == nullptr)
    title = "Simulation result - " + session->fileName() + " (no detector)";
  else
    title = "Simulation result - "
        + session->fileName()
        + " ("
        + QString::fromStdString(detector->name())
        + ")";
  setWindowTitle(title);
  refreshUi();
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

void
DetectorWindow::showEvent(QShowEvent *)
{
  refreshUi();
}

void
DetectorWindow::onViewChanged()
{
  refreshUi();
}

void
DetectorWindow::onScrollBarsChanged()
{
  QPointF p = QPointF(
        ui->horizontalScrollBar->value(),
        ui->verticalScrollBar->value()) / m_navWidget->zoom();

  m_navWidget->setCurrPoint(p);
}

void
DetectorWindow::onClearDetector()
{
  if (m_detector != nullptr) {
    m_detector->clear();
    m_navWidget->recalcImage();
    m_navWidget->update();
  }
}
