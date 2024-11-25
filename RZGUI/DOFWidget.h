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

#ifndef DOFWIDGET_H
#define DOFWIDGET_H

#include <QWidget>
#include <SimulationSession.h>
#include <QMap>

namespace Ui {
  class DOFWidget;
}

class DOFAdjustWidget;

class DOFWidget : public QWidget
{
  Q_OBJECT

  SimulationSession *m_session = nullptr;
  QMap<QString, DOFAdjustWidget *> m_dofToWidget;

  void makeWidgets();

public:
  explicit DOFWidget(SimulationSession *session, QWidget *parent = nullptr);
  ~DOFWidget();

public slots:
  void onDOFChanged(qreal);

signals:
  void dofChanged();

private:
  Ui::DOFWidget *ui;
};

#endif // DOFWIDGET_H
