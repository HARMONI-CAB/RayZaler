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
