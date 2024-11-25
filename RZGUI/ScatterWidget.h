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

#ifndef SCATTERWIDGET_H
#define SCATTERWIDGET_H

#include <QWidget>
#include <QThread>
#include <QFont>
#include <QFontMetrics>
#include "DataProductWidget.h"
#include <DataProducts/Scatter.h>

class ScatterAsyncRenderer;

#define SCATTER_WIDGET_ASYNC_THRESHOLD 50000

class ScatterWidget : public DataProductWidget
{
  Q_OBJECT

protected:
  virtual AsyncDataProductRenderer *makeRenderer(RZ::DataProduct *) override;

public:
  ScatterWidget(RZ::ScatterDataProduct *prod, QWidget *parent = nullptr);
  virtual ~ScatterWidget() override;

  void addSet(RZ::ScatterSet *);
};

#endif // SCATTERWIDGET_H
