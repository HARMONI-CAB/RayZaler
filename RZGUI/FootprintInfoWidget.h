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

#ifndef FOOTPRINTINFOWIDGET_H
#define FOOTPRINTINFOWIDGET_H

#include <QWidget>

namespace Ui {
  class FootprintInfoWidget;
}

struct SurfaceFootprint;

class FootprintInfoWidget : public QWidget
{
  Q_OBJECT

  void connectAll();

public:
  explicit FootprintInfoWidget(SurfaceFootprint const *, QWidget *parent = nullptr);
  virtual ~FootprintInfoWidget() override;

  void setFootprint(SurfaceFootprint const *);

public slots:
  void onToggleShowHide();

private:
  Ui::FootprintInfoWidget *ui;

};

#endif // FOOTPRINTINFOWIDGET_H