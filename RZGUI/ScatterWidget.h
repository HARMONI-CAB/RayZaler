#ifndef SCATTERWIDGET_H
#define SCATTERWIDGET_H

#include <QWidget>
#include <QThread>
#include <QFont>
#include <QFontMetrics>
#include "DataProductWidget.h"
#include <DataProducts/Scatter.h>

class ScatterTree;
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
};

#endif // SCATTERWIDGET_H
