#include "SpotDiagramWindow.h"
#include "ui_SpotDiagramWindow.h"
#include "ScatterWidget.h"
#include "GUIHelpers.h"
#include <OpticalElement.h>

SpotDiagramWindow::SpotDiagramWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::SpotDiagramWindow)
{
  ui->setupUi(this);


  m_product = new RZ::ScatterDataProduct("Example product");

  auto surface = new RZ::OpticalSurface;

  surface->name = "Example surface";

  surface->locationArray.clear();
  for (unsigned i = 0; i < 100000; ++i) {
    RZ::Real rho = 1e-1 + 1e-1 * rand() / (RZ::Real) RAND_MAX;
    RZ::Real phi = 2 * M_PI * rand() / (RZ::Real) RAND_MAX;

    surface->locationArray.push_back(rho * cos(phi));
    surface->locationArray.push_back(rho * sin(phi));
    surface->locationArray.push_back(0);
  }

  surface->hits.resize(surface->locationArray.size() / 3);

  m_product->addSurface(0xffff0000, surface, "Beam one");

  surface->locationArray.clear();
  for (unsigned i = 0; i < 100000; ++i) {
    RZ::Real rho = 1e-1 + 1e-2 * rand() / (RZ::Real) RAND_MAX;
    RZ::Real phi = 2 * M_PI * rand() / (RZ::Real) RAND_MAX;

    surface->locationArray.push_back(rho * cos(phi));
    surface->locationArray.push_back(rho * sin(phi));
    surface->locationArray.push_back(0);
  }

  surface->hits.resize(surface->locationArray.size() / 3);

  m_product->addSurface(0xff0000ff, surface, "Beam two");

  m_widget  = new ScatterWidget(m_product);

  ui->centerGrid->addWidget(m_widget, 0, 0, 2, 1);

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
