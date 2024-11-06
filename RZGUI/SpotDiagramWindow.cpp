#include "SpotDiagramWindow.h"
#include "ui_SpotDiagramWindow.h"
#include "ScatterWidget.h"
#include "GUIHelpers.h"
#include <OpticalElement.h>

SpotDiagramWindow::SpotDiagramWindow(RZ::ScatterDataProduct *product, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::SpotDiagramWindow)
{
  ui->setupUi(this);

  m_product = product;
  m_widget  = new ScatterWidget(m_product);

  ui->centerGrid->addWidget(m_widget, 0, 0, 2, 1);

  setWindowTitle(QString::fromStdString(product->productName()));

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
