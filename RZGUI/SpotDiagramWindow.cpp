#include "SpotDiagramWindow.h"
#include "ui_SpotDiagramWindow.h"
#include "ScatterWidget.h"
#include "GUIHelpers.h"
#include <OpticalElement.h>
#include <DataProducts/Scatter.h>

SpotDiagramWindow::SpotDiagramWindow(QString title, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::SpotDiagramWindow)
{
  ui->setupUi(this);

  m_product = new RZ::ScatterDataProduct(title.toStdString());
  m_widget  = new ScatterWidget(m_product);

  ui->centerGrid->addWidget(m_widget, 0, 0, 2, 1);

  setWindowTitle("Footprint - " + QString::fromStdString(m_product->productName()));

  legend();
}

SpotDiagramWindow::~SpotDiagramWindow()
{
  delete ui;
}

void
SpotDiagramWindow::legend()
{
  QString html;
  auto &sets = m_product->dataSets();
  bool first = true;

  for (auto &set : sets) {
    if (first)
      first = false;
    else
      html += "<br />\n";

    html += "<b style=\"color: " + argbToCss(set->id()) + "\"> ▬ </b>";
    html += QString::fromStdString(set->label());
  }

  ui->legendLabel->setText(html);
}

void
SpotDiagramWindow::updateView()
{
  m_widget->updateView();
  legend();
}

void
SpotDiagramWindow::resetZoom()
{
  m_widget->resetZoom();
}

void
SpotDiagramWindow::transferFootprint(SurfaceFootprint &footprint)
{
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

      curve.xydata.push_back(QPointF(x, y));
    }

    N += points;

    m_widget->addCurve(curve);
  }

  x0 /= static_cast<qreal>(N);
  y0 /= static_cast<qreal>(N);

  m_widget->setResetZoom(1. / (3 * maxAbsX0), x0, y0);
}
