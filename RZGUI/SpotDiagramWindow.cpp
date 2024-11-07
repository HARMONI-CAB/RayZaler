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
