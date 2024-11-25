//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

#include "SpotDiagramWindow.h"
#include "ui_SpotDiagramWindow.h"
#include "ScatterWidget.h"
#include "GUIHelpers.h"
#include <Helpers.h>
#include <OpticalElement.h>
#include <DataProducts/Scatter.h>
#include <QFileDialog>
#include "FootprintInfoWidget.h"
#include <QResizeEvent>
#include <QMessageBox>

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

  connect(
        ui->splitter,
        SIGNAL(splitterMoved(int, int)),
        this,
        SLOT(onSplitterMoved()));
}

void
SpotDiagramWindow::resizeEvent(QResizeEvent *ev)
{
  QList<int> sizes;
  sizes << (ev->size().width() - m_legendWidth) << m_legendWidth;
  ui->splitter->setSizes(sizes);
}

bool
SpotDiagramWindow::saveToFile(QString path)
{
  std::string strPath = path.toStdString();
  FILE *oFile = nullptr;
  bool ok = false;

  try {
    if ((oFile = fopen(strPath.c_str(), "wb")) == nullptr)
      throw std::runtime_error(
            string_printf(
              "Cannot open selected file for writing: %s",
              strerror(errno)));
#define FULLFMT "%+23.15e"

    std::string line;
    line =
        string_printf(
          "%10s, %23s, %23s, %23s, %23s, %23s, %23s, "
          "%20s, %10s, %10s, %10s\n",
          "Ray id", "pX", "pY", "pZ", "uX", "uY", "uZ",
          "label", "Beam id", "Transmitted", "Vignetted");

    if (fwrite(line.c_str(), line.size(), 1, oFile) < 1)
      throw std::runtime_error(
          string_printf(
            "Failed to write ray data: %s",
            strerror(errno)));

    for (auto &fp : m_footprints) {
      for (unsigned int i = 0; i < fp.locations.size() / 3; ++i) {
        line = string_printf(
              "%10u, "                                // Ray index
              FULLFMT ", " FULLFMT ", " FULLFMT ", "  // Location
              FULLFMT ", " FULLFMT ", " FULLFMT ",",  // Direction
              i,
              fp.locations[3 * i + 0],
              fp.locations[3 * i + 1],
              fp.locations[3 * i + 2],
              fp.directions[3 * i + 0],
              fp.directions[3 * i + 1],
              fp.directions[3 * i + 2]);
        if (i == 0) {
          line += string_printf(
                " %20s, "   // Label
                "%10u, "   // Beam ID
                "%11lu, "  // Transmitted
                "%10lu",   // Vignetted
                fp.label.c_str(),
                fp.id,
                fp.transmitted,
                fp.vignetted);
        } else {
          line += ",,,";
        }

        line += "\n";

        if (fwrite(line.c_str(), line.size(), 1, oFile) < 1)
          throw std::runtime_error(
              string_printf(
                "Failed to write ray data: %s",
                strerror(errno)));
      }
    }
    ok = true;
  } catch (std::runtime_error const &e) {
    QMessageBox::critical(this, "Save data to file", e.what());
  }
#undef FULLFMT

  if (oFile != nullptr)
    fclose(oFile);

  return ok;
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
SpotDiagramWindow::transferFootprint(SurfaceFootprint &newFootprint)
{
  m_footprints.push_back(std::move(newFootprint));

  auto &footprint = m_footprints.back();

  auto infoWidget = new FootprintInfoWidget(&footprint, this);

  ui->verticalLayout->insertWidget(m_infoWidgets.size(), infoWidget);

  m_infoWidgets.push_back(infoWidget);

  RZ::ScatterSet *set = new RZ::ScatterSet(
        footprint.color,
        footprint.locations,
        footprint.label,
        3,                        // Stride is 3 (3D vectors)
        false);                   // Do transfer
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
  for (auto &p: m_infoWidgets) {
    ui->verticalLayout->removeWidget(p);
    p->deleteLater();
  }

  m_infoWidgets.clear();
  m_footprints.clear();

  emit clear();
}

void
SpotDiagramWindow::onSaveData()
{
  if (m_saveDialog->exec() && !m_saveDialog->selectedFiles().empty())
    saveToFile(m_saveDialog->selectedFiles()[0]);
}

void
SpotDiagramWindow::onSplitterMoved()
{
  auto sizes = ui->splitter->sizes();

  m_legendWidth = sizes.at(1);
}
