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

#ifndef DOFADJUSTWIDGET_H
#define DOFADJUSTWIDGET_H

#include <QWidget>
#include <Recipe.h>
#include <GenericCompositeModel.h>

namespace Ui {
  class DOFAdjustWidget;
}

enum DOFWidgetType {
  DOF_WT_NONE,
  DOF_WT_RANGE,
  DOF_WT_POSITIVE,
  DOF_WT_FREE
};

class QLineEdit;

class DOFAdjustWidget : public QWidget
{
  Q_OBJECT

  QString m_name;
  DOFWidgetType m_type = DOF_WT_NONE;
  Ui::DOFAdjustWidget *ui;

  qreal m_currValue;

  RZ::GenericModelParam *m_currentParam = nullptr;

  void adjustUi();
  void refreshUi();
  void connectAll();

  qreal fromRange() const;
  void toRange(qreal);

  qreal processTextValue(QLineEdit *);

public:
  explicit DOFAdjustWidget(QWidget *parent = nullptr);
  ~DOFAdjustWidget();

  void setValue(qreal);
  qreal value() const;

  void setName(QString const &);
  QString name() const;

  void setModelParam(RZ::GenericModelParam *param);
  RZ::GenericModelParam *modelParam();

signals:
  void valueChanged(qreal);

public slots:
  void onSliderChanged();
  void onReset();
  void onValueChanged();
  void onDouble();
  void onHalf();
  void onZero();
};

#endif // DOFADJUSTWIDGET_H
