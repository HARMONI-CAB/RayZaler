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
