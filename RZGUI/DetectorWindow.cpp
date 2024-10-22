#include "DetectorWindow.h"
#include "ui_DetectorWindow.h"
#include "ImageNavWidget.h"
#include "SimulationSession.h"
#include "GUIHelpers.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFrame>

DetectorWindow::DetectorWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::DetectorWindow)
{
  ui->setupUi(this);

  m_navWidget = new ImageNavWidget();
  m_navWidget->setAutoScale(true);

  ui->viewGrid->addWidget(m_navWidget, 1, 0);

  m_saveDialog = new QFileDialog(this);
  m_saveDialog->setWindowTitle("Export detector data");
  m_saveDialog->setFileMode(QFileDialog::AnyFile);
  m_saveDialog->setAcceptMode(QFileDialog::AcceptSave);
  m_saveDialog->setNameFilter(
    "Normalized PNG image (*.png);;"
    "Raw counts (*.raw);;"
    "Complex float64 amplitude (*.bin)");
  
  populateDetectorMenu();
  populateStatusBar();

  connectAll();
}

#define MAKE_STATUS_LABEL(name)            \
do {                                       \
  name = new QLabel;                       \
  name->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); \
  name->setAlignment(Qt::AlignRight); \
  ui->statusbar->addPermanentWidget(name); \
} while(false)

void
DetectorWindow::populateStatusBar()
{
  MAKE_STATUS_LABEL(m_pxSizeLabel);
  m_pxSizeLabel->setFrameShape(QFrame::StyledPanel);

  MAKE_STATUS_LABEL(m_detSizeLabel);
  m_detSizeLabel->setFrameShape(QFrame::StyledPanel);

  MAKE_STATUS_LABEL(m_pixelsLabel);
  m_pixelsLabel->setFrameShape(QFrame::StyledPanel);

  MAKE_STATUS_LABEL(m_rangeLabel);
  m_rangeLabel->setFrameShape(QFrame::StyledPanel);

  QLabel *spacer = new QLabel;
  QLabel *cLabel, *xyLabel;

  ui->statusbar->addPermanentWidget(spacer, 1);
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);


  MAKE_STATUS_LABEL(m_posLabel);

  MAKE_STATUS_LABEL(cLabel);
  MAKE_STATUS_LABEL(m_countsLabel);

  MAKE_STATUS_LABEL(xyLabel);
  MAKE_STATUS_LABEL(m_pxLabel);

  cLabel->setText("C:");
  xyLabel->setText("XY:");
}

#undef MAKE_STATUS_LABEL

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

    m_pxSizeLabel->setText(
          QString::asprintf(
            "P: %g µm×%g µm",
            m_detector->pxWidth() * 1e6,
            m_detector->pxHeight() * 1e6));

    m_detSizeLabel->setText(
          QString::asprintf(
            "D: %g mm×%g mm",
            m_detector->width()  * 1e3,
            m_detector->height() * 1e3));

    m_pixelsLabel->setText(
          QString::asprintf(
            "G: %d×%d",
            m_detector->cols(),
            m_detector->rows()));

    m_rangeLabel->setText(
          QString::asprintf("R: [%g, %g]",
            m_navWidget->imgMin(),
            m_navWidget->imgMax()));
  } else {
    m_pxSizeLabel->setText("P: N/A");
    m_detSizeLabel->setText("D: N/A");
    m_pixelsLabel->setText("G: N/A");
    m_rangeLabel->setText("R: N/A");
  }

  m_posLabel->setText("M: N/A");
  m_pxLabel->setText("N/A");
  m_countsLabel->setText("N/A");
}

void
DetectorWindow::refreshUi()
{
  if (m_detector != nullptr) {
    qreal zoom    = m_navWidget->zoom();
    auto imgSize  = QSizeF(m_navWidget->imageSize()) * zoom;
    auto viewSize = QSizeF(ui->viewFrame->size());

    int currScrollX = ui->horizontalScrollBar->value();
    int currScrollY = ui->verticalScrollBar->value();

    BLOCKSIG_BEGIN(ui->horizontalScrollBar);
    if (viewSize.width() < imgSize.width()) {
      auto horizontalRange = static_cast<int>(
            imgSize.width() - viewSize.width());

      ui->horizontalScrollBar->setMinimum(-horizontalRange / 2);
      ui->horizontalScrollBar->setMaximum(
            horizontalRange - horizontalRange / 2);


      ui->horizontalScrollBar->setPageStep(static_cast<int>(viewSize.width()));
      ui->horizontalScrollBar->setValue(currScrollX);
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
      ui->verticalScrollBar->setValue(currScrollY);
      ui->verticalScrollBar->setEnabled(true);
    } else {
      ui->verticalScrollBar->setRange(0, 0);
      ui->verticalScrollBar->setEnabled(false);
    }
    BLOCKSIG_END();

    BLOCKSIG(ui->actionShow_photons, setChecked(m_showPhotons));
    BLOCKSIG(ui->actionElectricField, setChecked(!m_showPhotons));
  }

  BLOCKSIG(ui->actionShow_photons, setEnabled(m_detector));
  BLOCKSIG(ui->actionElectricField, setEnabled(m_detector));
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

  connect(
        ui->actionElectricField,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onChangeDetectorRep()));

  connect(
        ui->actionShow_photons,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onChangeDetectorRep()));

  connect(
        ui->actionTogglePhase,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onToggleShowPhase()));

  connect(
        ui->actionToggleGrid,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onToggleGrid()));

  connect(
        ui->actionExportAs,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onExport()));
}

void
DetectorWindow::setDetector(RZ::Detector *detector)
{
  QString title;

  m_detector = detector;
  m_navWidget->setDetector(detector);

  if (detector == nullptr) {
    if (m_session != nullptr)
      title = "Simulation result - " + m_session->fileName() + " (no detector)";
    else
      title = "Simulation result - no simulation session";
    // TODO: Set detector name somewhere
  } else {
    auto detName = QString::fromStdString(detector->name());
    // TODO: Set detector name somewhere
    title = "Simulation result - "
        +  m_session->fileName()
        + " ("
        + detName
        + ")";
    m_navWidget->setShowPhotons(m_showPhotons);

    fixLabelSizeToContents(
          m_countsLabel,
          QString::number(qBound(100, SCAST(int, m_navWidget->imgMax()), 10000000)));

    fixLabelSizeToContents(
          m_pxLabel,
          QString::number(m_detector->cols()) + ", " + QString::number(m_detector->rows()));
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
  int x = ui->horizontalScrollBar->value();
  int y = ui->verticalScrollBar->value();

  m_scrollDx = x - m_lastX;
  m_scrollDy = y - m_lastY;

  QPointF p = m_navWidget->currPoint();

  p.setX(p.x() - m_scrollDx);
  p.setY(p.y() - m_scrollDy);

  m_navWidget->setCurrPoint(p);

  m_lastX = x;
  m_lastY = y;
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
    qreal posX = (loc.x()) * m_detector->pxWidth();
    qreal posY = (loc.y()) * m_detector->pxHeight();

    int pixelX = SCAST(int, floor(loc.x() + .5 * m_detector->cols()));
    int pixelY = SCAST(int, floor(loc.y() + .5 * m_detector->rows()));

    if (pixelX < 0 || pixelX >= SCAST(int, m_detector->cols())
        || pixelY < 0 || pixelY >= SCAST(int, m_detector->rows())) {
      m_pxLabel->setText("N/A");
      m_countsLabel->setText("N/A");
    } else {
      int index = pixelX + pixelY * SCAST(int, m_detector->stride());
      m_pxLabel->setText(QString::asprintf("%d, %d", pixelX, pixelY));
      m_countsLabel->setText(QString::asprintf("%d", m_detector->data()[index]));
    }

    m_posLabel->setText(QString::asprintf("M: %+g mm, %+g mm", posX * 1e3, posY * 1e3));
  }
}

void
DetectorWindow::onChangeDetectorRep()
{
  QObject *clicked = QObject::sender();

  if (clicked == qobject_cast<QObject *>(ui->actionShow_photons))
    m_showPhotons = ui->actionShow_photons->isChecked();

  if (clicked == qobject_cast<QObject *>(ui->actionElectricField))
    m_showPhotons = !ui->actionElectricField->isChecked();

  m_navWidget->setShowPhotons(m_showPhotons);
  refreshUi();
}

void
DetectorWindow::onToggleShowPhase()
{
  m_navWidget->setShowPhase(ui->actionTogglePhase->isChecked());
  refreshUi();
}

void
DetectorWindow::onToggleGrid()
{
  m_navWidget->setShowGrid(ui->actionToggleGrid->isChecked());
  refreshUi();
}

void
DetectorWindow::onExport()
{
  if (m_detector != nullptr) {
    bool ok = false;

    if (m_saveDialog->exec() && !m_saveDialog->selectedFiles().empty()) {
      QString fileName = m_saveDialog->selectedFiles()[0];
      auto filter      = m_saveDialog->selectedNameFilter();
      std::string path = fileName.toStdString();

      if (filter.contains("*.png"))
        ok = m_detector->savePNG(path);
      else if (filter.contains("*.raw"))
        ok = m_detector->saveRawData(path);
      else if (filter.contains("*.bin"))
        ok = m_detector->saveAmplitude(path);

      if (!ok) {
        QMessageBox::critical(
          this,
          "Export data",
          "Cannot export current detector state. Open log window for details");
      }
    }
  }
}
