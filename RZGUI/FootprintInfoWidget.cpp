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

#include "FootprintInfoWidget.h"
#include "ui_FootprintInfoWidget.h"
#include "SpotDiagramWindow.h"
#include "GUIHelpers.h"
#include <Helpers.h>

FootprintInfoWidget::FootprintInfoWidget(
    SurfaceFootprint const *fp,
    QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FootprintInfoWidget)
{
  ui->setupUi(this);

  if (fp != nullptr)
    setFootprint(fp);

  connectAll();
  onToggleShowHide();
}

FootprintInfoWidget::~FootprintInfoWidget()
{
  delete ui;
}

void
FootprintInfoWidget::connectAll()
{
  connect(
        ui->showHideButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onToggleShowHide()));
}

void
FootprintInfoWidget::setFootprint(SurfaceFootprint const *fp)
{
  auto color = QColor::fromRgb(fp->color);
  const RZ::Vec3 *asVec3 = reinterpret_cast<const RZ::Vec3 *>(fp->locations.data());
  qreal maxRad = 0;
  qreal rmsRad = 0;
  qreal R2     = 0;
  size_t N     =  fp->locations.size() / 3;

  RZ::Vec3 center = sumPrecise<RZ::Vec3>(asVec3, N);

  qreal x0 = center.x / N;
  qreal y0 = center.y / N;
  size_t totalRays = fp->transmitted + fp->vignetted;

  ui->chiefCenterLabel->setText(
        toSensibleUnits(fp->locations[0])
        + ", "
        + toSensibleUnits(fp->locations[1]));

  if (totalRays == 0) {
    ui->totalRaysLabel->setText("None");
    ui->vignettedRaysLabel->setText("None");
    ui->estimatedFLabel->setText("N/A");
    ui->centerLabel->setText("N/A");
    ui->maxRadiusLabel->setText("N/A");
    ui->rmsRadiusLabel->setText("N/A");
  } else {
    qreal vignRate =
        1e2 * static_cast<qreal>(fp->vignetted)
        / static_cast<qreal>(totalRays);
    qreal corr = 0, c = 0, t;
    qreal fNum = std::numeric_limits<qreal>::infinity();

    ui->totalRaysLabel->setText(QString::number(totalRays));
    ui->vignettedRaysLabel->setText(
          QString::asprintf("%ld (%g%%)", fp->vignetted, vignRate));

    ui->showHideButton->setStyleSheet("background-color: " + color.name());
    ui->fooprintNameLabel->setText(QString::fromStdString(fp->label));
    ui->centerLabel->setText(toSensibleUnits(x0) + ", " + toSensibleUnits(y0));

    RZ::Vec3 chiefRayDirection(fp->directions.data());

    for (size_t i = 3; i < fp->locations.size(); i += 3) {
      qreal x = fp->locations[i] - x0;
      qreal y = fp->locations[i + 1] - y0;

      R2 = x * x + y * y;
      maxRad = fmax(R2, maxRad);
      fNum   = RZ::fabsmin(.5 / tan(acos(RZ::Vec3(fp->directions.data() + i) * chiefRayDirection)), fNum);
      
      corr = R2 - c;
      t = rmsRad + corr;
      c = (t - rmsRad) - corr;
      rmsRad = t;
    }

    rmsRad = sqrt(rmsRad / static_cast<qreal>(N));
    maxRad = sqrt(maxRad);

    ui->maxRadiusLabel->setText(toSensibleUnits(maxRad));
    ui->rmsRadiusLabel->setText(toSensibleUnits(rmsRad));

    if (fabs(fNum) > 1e14)
      ui->estimatedFLabel->setText("(collimated)");
    else
      ui->estimatedFLabel->setText(QString::asprintf("f/%.4g", fNum));
  }
}


void
FootprintInfoWidget::onToggleShowHide()
{
  ui->rayPropertiesFrame->setVisible(ui->showHideButton->isChecked());
}
