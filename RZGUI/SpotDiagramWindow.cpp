#include "SpotDiagramWindow.h"
#include "ui_SpotDiagramWindow.h"
#include "ScatterWidget.h"
#include "GUIHelpers.h"
#include <OpticalElement.h>
#include <DataProducts/Scatter.h>
#include <QFileDialog>
#include "FootprintInfoWidget.h"

SpotDiagramWindow::SpotDiagramWindow(QString title, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::SpotDiagramWindow)
{
  ui->setupUi(this);

  m_product = new RZ::ScatterDataProduct(title.toStdString());
  m_widget  = new ScatterWidget(m_product);

  m_saveDialog = new QFileDialog(this);
  m_saveDialog->setWindowTitle("Save data");
  m_saveDialog->setFileMode(QFileDialog::AnyFile);
  m_saveDialog->setAcceptMode(QFileDialog::AcceptSave);
  m_saveDialog->setNameFilter(
    "Comma-separated values (*.csv);;"
    "All files (*)");


  ui->splitter->replaceWidget(0, m_widget);

  setWindowTitle(QString::fromStdString(m_product->productName()) + " - Footprint");

  delete ui->label;

  connectAll();
}

SpotDiagramWindow::~SpotDiagramWindow()
{
  delete ui;
}

void
SpotDiagramWindow::connectAll()
{
  connect(
        ui->actionResetZoom,
        SIGNAL(triggered(bool)),
        this,
        SLOT(resetZoom()));

  connect(
        ui->actionSaveData,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onSaveData()));

  connect(
        ui->actionClear,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onClear()));

  connect(
        this,
        SIGNAL(clear()),
        m_widget,
        SIGNAL(clearData()));

  connect(
        this,
        SIGNAL(saveData(QString)),
        m_widget,
        SIGNAL(saveData(QString)));
}

void
SpotDiagramWindow::updateView()
{
  m_widget->updateView();
}

void
SpotDiagramWindow::resetZoom()
{
  m_widget->resetZoom();
}

void
SpotDiagramWindow::transferFootprint(SurfaceFootprint &footprint)
{
  auto infoWidget = new FootprintInfoWidget(&footprint, this);

  ui->verticalLayout->insertWidget(m_infoWidgets.size(), infoWidget);

  m_infoWidgets.push_back(infoWidget);

  RZ::ScatterSet *set = new RZ::ScatterSet(
        footprint.color,
        footprint.locations,
        footprint.label,
        3,                        // Stride is 3 (3D vectors)
        true);                    // Do transfer
  m_widget->addSet(set);
}

void
SpotDiagramWindow::setEdges(std::vector<std::vector<RZ::Real>> const &edges)
{
  qreal x0 = 0;
  qreal y0 = 0;
  qreal maxAbsX0 = 1e-9;
  size_t N = 0;

  m_widget->clearCurves();

  for (auto &edge: edges) {
    DataProductCurve curve;
    auto points = edge.size() / 3;

    curve.width = 2;
    curve.color = QColor::fromRgb(0, 0, 0);
    curve.closed = true;

    for (size_t i = 0; i < points; ++i) {
      auto x = edge[3 * i + 0];
      auto y = edge[3 * i + 1];

      x0 += x;
      y0 += y;

      if (fabs(x) > maxAbsX0)
        maxAbsX0 = x;
      if (fabs(y) > maxAbsX0)
        maxAbsX0 = y;

      curve.xydata.push_back(QPointF(x, y));
    }

    N += points;

    m_widget->addCurve(curve);
  }

  x0 /= static_cast<qreal>(N);
  y0 /= static_cast<qreal>(N);

  m_widget->setResetZoom(1. / (2.5 * maxAbsX0), x0, y0);
}

void
SpotDiagramWindow::onClear()
{
  int i;

  for (auto &p: m_infoWidgets) {
    ui->verticalLayout->removeWidget(p);
    p->deleteLater();
  }

  m_infoWidgets.clear();

  emit clear();
}

void
SpotDiagramWindow::onSaveData()
{
  if (m_saveDialog->exec() && !m_saveDialog->selectedFiles().empty())
    emit saveData(m_saveDialog->selectedFiles()[0]);
}