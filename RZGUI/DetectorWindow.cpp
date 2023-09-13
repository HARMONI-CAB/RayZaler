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

  ui->viewGrid->addWidget(m_navWidget, 1, 0);

  populateDetectorMenu();
  connectAll();
}

DetectorWindow::~DetectorWindow()
{
  delete ui;
}

void
DetectorWindow::populateDetectorMenu()
{
  // Clear menu
  ui->menuDetectorChange->clear();

  for (auto p : m_detectorActions)
    p->deleteLater();

  m_detectorActions.clear();

  if (m_session != nullptr) {
    for (auto p : m_session->topLevelModel()->detectors()) {
      QString detName = QString::fromStdString(p);
      QAction *action = new QAction(this);
      action->setCheckable(true);
      action->setText(detName);
      action->setData(detName);
      m_detectorActions.push_back(action);
      ui->menuDetectorChange->addAction(action);

      connect(
            action,
            SIGNAL(triggered(bool)),
            this,
            SLOT(onChangeDetector()));
    }
  }

  if (m_detectorActions.size() == 0)
    ui->menuDetectorChange->addAction(ui->action_No_detector);
}
void
DetectorWindow::refreshDetectorParams()
{
  if (m_detector != nullptr) {
    for (auto p : m_detectorActions) {
      QString name = p->data().toString();
      std::string stdName = name.toStdString();

      p->setChecked(stdName == m_detector->name());
    }

    ui->pxWidthLabel->setText(
          QString::number(m_detector->pxWidth() * 1e6) + " µm");
    ui->pxHeightLabel->setText(
          QString::number(m_detector->pxHeight() * 1e6) + " µm");
    ui->widthLabel->setText(
          QString::number(m_detector->width() * 1e3) + " mm");
    ui->heightLabel->setText(
          QString::number(m_detector->height() * 1e3) + " mm");
    ui->horizontalPixelsLabel->setText(
          QString::number(m_detector->cols()) + " px");
    ui->verticalPixelsLabel->setText(
          QString::number(m_detector->rows()) + " px");
    ui->rangeLabel->setText(
          QString::asprintf("[%g, %g]",
          m_navWidget->imgMin(),
          m_navWidget->imgMax()));
  } else {
    ui->pxWidthLabel->setText("N/A");
    ui->pxHeightLabel->setText("N/A");
    ui->widthLabel->setText("N/A");
    ui->heightLabel->setText("N/A");
    ui->horizontalPixelsLabel->setText("N/A");
    ui->verticalPixelsLabel->setText("N/A");
    ui->rangeLabel->setText("N/A");
  }

  ui->posXLabel->setText("N/A");
  ui->posYLabel->setText("N/A");

  ui->pixelXLabel->setText("N/A");
  ui->pixelYLabel->setText("N/A");

  ui->countsLabel->setText("N/A");
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
        m_navWidget,
        SIGNAL(mouseMoved(QPointF)),
        this,
        SLOT(onHoverPixel(QPointF)));

  connect(
        ui->actionClear,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onClearDetector()));

  connect(
        ui->actionLogScale,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onToggleLogScale()));

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
DetectorWindow::setDetector(RZ::Detector *detector)
{
  QString title;

  m_detector = detector;
  m_navWidget->setDetector(detector);

  if (detector == nullptr) {
    title = "Simulation result - " + m_session->fileName() + " (no detector)";
    ui->detGroupBox->setTitle("No detector");
  } else {
    auto detName = QString::fromStdString(detector->name());
    ui->detGroupBox->setTitle(detName);
    title = "Simulation result - "
        +  m_session->fileName()
        + " ("
        + detName
        + ")";
  }

  setWindowTitle(title);
  refreshDetectorParams();
  refreshUi();
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

  populateDetectorMenu();
  setDetector(detector);
}

void
DetectorWindow::refreshImage()
{
  m_navWidget->recalcImage();
  m_navWidget->update();
  refreshDetectorParams();
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

void
DetectorWindow::onToggleLogScale()
{
  m_navWidget->setLogScale(ui->actionLogScale->isChecked());
}

void
DetectorWindow::onChangeDetector()
{
  QAction *action = SCAST(QAction *, QObject::sender());
  QString name = action->data().toString();
  std::string stdName = name.toStdString();

  auto model = m_session->topLevelModel();

  setDetector(model->lookupDetector(stdName));
}

void
DetectorWindow::onHoverPixel(QPointF loc)
{
  if (m_detector != nullptr) {
    qreal posX = (loc.x() + .5) * m_detector->width();
    qreal posY = (loc.y() + .5) * m_detector->height();

    int pixelX = SCAST(int, loc.x()) + SCAST(int, m_detector->cols()) / 2;
    int pixelY = SCAST(int, loc.y()) + SCAST(int, m_detector->rows()) / 2;

    if (pixelX < 0 || pixelX >= SCAST(int, m_detector->cols())
        || pixelY < 0 || pixelY >= SCAST(int, m_detector->rows())) {
      ui->pixelXLabel->setText("N/A");
      ui->pixelYLabel->setText("N/A");
      ui->countsLabel->setText("N/A");
    } else {
      int index = pixelX + pixelY * SCAST(int, m_detector->stride());
      ui->pixelXLabel->setText(QString::number(pixelX));
      ui->pixelYLabel->setText(QString::number(pixelY));
      ui->countsLabel->setText(QString::number(m_detector->data()[index]));
    }

    ui->posXLabel->setText(QString::number(posX * 1e3) + " mm");
    ui->posYLabel->setText(QString::number(posY * 1e3) + " mm");
  }
}
