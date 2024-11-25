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

#include "DOFWidget.h"
#include "ui_DOFWidget.h"
#include "DOFAdjustWidget.h"

DOFWidget::DOFWidget(SimulationSession *sess, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::DOFWidget)
{
  ui->setupUi(this);

  m_session = sess;

  makeWidgets();
}

DOFWidget::~DOFWidget()
{
  delete ui;
}

void
DOFWidget::makeWidgets()
{
  auto topLevel = m_session->topLevelModel();

  for (auto &dof : topLevel->dofs()) {
    auto param = topLevel->lookupDof(dof);

    if (param != nullptr) {
      auto widget = new DOFAdjustWidget;

      auto name = QString::fromStdString(dof);
      widget->setName(name);
      widget->setModelParam(param);

      m_dofToWidget[name] = widget;

      connect(
            widget,
            SIGNAL(valueChanged(qreal)),
            this,
            SLOT(onDOFChanged(qreal)));
    }
  }

  if (!m_dofToWidget.empty()) {
    // Remove placeholder
    ui->verticalLayout_2->removeWidget(ui->label);
    ui->label->deleteLater();

    int count = 0;

    for (auto entry : m_dofToWidget)
      ui->verticalLayout_2->insertWidget(count++, entry);
  }
}

void
DOFWidget::onDOFChanged(qreal value)
{
  DOFAdjustWidget *widget = qobject_cast<DOFAdjustWidget *>(QObject::sender());

  m_session->topLevelModel()->setDof(
        widget->name().toStdString(),
        value);

  emit dofChanged();
}
