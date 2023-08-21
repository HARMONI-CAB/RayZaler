#include "SessionTabWidget.h"
#include "SimulationSession.h"
#include "ui_SessionTabWidget.h"

#include "RZGUIGLWidget.h"


SessionTabWidget::SessionTabWidget(
    SimulationSession *session,
    QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SessionTabWidget)
{
  m_session = session;

  ui->setupUi(this);

  m_glWidget = new RZGUIGLWidget(this);
  ui->mainGrid->addWidget(m_glWidget, 0, 0, 1, 1);

  m_glWidget->setModel(session->topLevelModel());
}

SessionTabWidget::~SessionTabWidget()
{
  delete ui;
}
