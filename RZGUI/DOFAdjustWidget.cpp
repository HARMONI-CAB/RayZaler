#include "DOFAdjustWidget.h"
#include "ui_DOFAdjustWidget.h"
#include <cmath>
#include <GUIHelpers.h>

DOFAdjustWidget::DOFAdjustWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::DOFAdjustWidget)
{
  ui->setupUi(this);

  connectAll();
}

DOFAdjustWidget::~DOFAdjustWidget()
{
  delete ui;
}

void
DOFAdjustWidget::connectAll()
{
  connect(
        ui->valueLineEdit,
        SIGNAL(editingFinished()),
        this,
        SLOT(onValueChanged()));

  connect(
        ui->positiveValueLine,
        SIGNAL(editingFinished()),
        this,
        SLOT(onValueChanged()));

  connect(
        ui->valueSlider,
        SIGNAL(valueChanged(int)),
        this,
        SLOT(onSliderChanged()));

  connect(
        ui->freeValReset,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onReset()));

  connect(
        ui->rangeReset,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onReset()));

  connect(
        ui->posResetButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onReset()));

  connect(
        ui->div2Button,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onHalf()));

  connect(
        ui->mul2Button,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onDouble()));

  connect(
        ui->zeroButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onZero()));
}

qreal
DOFAdjustWidget::fromRange() const
{
  auto min = m_currentParam->description->min;
  auto max = m_currentParam->description->max;

  qreal val = ui->valueSlider->value() / 100.;

  return min + (max - min) * val;
}

void
DOFAdjustWidget::toRange(qreal val)
{
  auto min = m_currentParam->description->min;
  auto max = m_currentParam->description->max;

  val -= min;
  val /= (max - min);
  val *= 100;

  BLOCKSIG(ui->valueSlider, setValue(static_cast<int>(val)));
}

void
DOFAdjustWidget::adjustUi()
{
  if (m_currentParam == nullptr) {
    ui->stackedWidget->setCurrentWidget(ui->undefinedPage);
    m_type = DOF_WT_NONE;
    return;
  }

  auto min = m_currentParam->description->min;
  auto max = m_currentParam->description->max;

  m_currValue = m_currentParam->value;

  if (!std::isinf(min) && !std::isinf(max)) {
    ui->stackedWidget->setCurrentWidget(ui->limitedRangePage);
    m_type = DOF_WT_RANGE;

    ui->minLabel->setText(asScientific(min));
    ui->maxLabel->setText(asScientific(max));
  } else if (min >= 0) {
    ui->stackedWidget->setCurrentWidget(ui->positiveValuePage);
    m_type = DOF_WT_POSITIVE;
  } else {
    ui->stackedWidget->setCurrentWidget(ui->freeRangepage);
    m_type = DOF_WT_FREE;
  }

  auto current = ui->stackedWidget->currentWidget();

  for (int i = 0; i < ui->stackedWidget->count(); ++i)
    if (ui->stackedWidget->widget(i) != current)
      ui->stackedWidget->widget(i)->setVisible(false);

  refreshUi();
}

qreal
DOFAdjustWidget::processTextValue(QLineEdit *edit)
{
  QString asText = edit->text();
  bool ok = false;

  qreal value = asText.toDouble(&ok);

  if (!ok || !m_currentParam->test(value))
    value = m_currValue;

  return value;
}

void
DOFAdjustWidget::refreshUi()
{
  switch (m_type) {
    case DOF_WT_NONE:
      break;

    case DOF_WT_RANGE:
      toRange(m_currValue);
      break;

    case DOF_WT_POSITIVE:
      BLOCKSIG(ui->positiveValueLine, setText(QString::number(m_currValue)));
      ui->zeroButton->setEnabled(m_currentParam->test(0));
      ui->mul2Button->setEnabled(m_currentParam->test(m_currValue * 2));
      ui->div2Button->setEnabled(m_currentParam->test(m_currValue / 2));
      break;

    case DOF_WT_FREE:
      BLOCKSIG(ui->valueLineEdit, setText(QString::number(m_currValue)));
      break;
  }

  ui->valueLabel->setText(asScientific(m_currValue));
}

void
DOFAdjustWidget::setValue(qreal val)
{
  if (m_currentParam == nullptr)
    return;

  if (!m_currentParam->test(val))
    return;

  if (!RZ::releq(m_currValue, val)) {
    m_currValue = val;
    refreshUi();

    emit valueChanged(val);
  }
}

qreal
DOFAdjustWidget::value() const
{
  return m_currValue;
}

void
DOFAdjustWidget::setName(QString const &name)
{
  m_name = name;
  ui->nameLabel->setText(name);
}

QString
DOFAdjustWidget::name() const
{
  return m_name;
}

void
DOFAdjustWidget::setModelParam(RZ::GenericModelParam *param)
{
  m_currentParam = param;

  adjustUi();
}

RZ::GenericModelParam *
DOFAdjustWidget::modelParam()
{
  return m_currentParam;
}

///////////////////////////////////// Slots ///////////////////////////////////
void
DOFAdjustWidget::onSliderChanged()
{
  setValue(fromRange());
}

void
DOFAdjustWidget::onReset()
{
  if (m_currentParam == nullptr)
    return;

  setValue(m_currentParam->description->defaultVal);
}

void
DOFAdjustWidget::onValueChanged()
{
  QLineEdit *edit = qobject_cast<QLineEdit *>(QObject::sender());

  setValue(processTextValue(edit));
}

void
DOFAdjustWidget::onDouble()
{
  setValue(m_currValue * 2);
}

void
DOFAdjustWidget::onHalf()
{
  setValue(m_currValue / 2);
}

void
DOFAdjustWidget::onZero()
{
  setValue(0);
}


