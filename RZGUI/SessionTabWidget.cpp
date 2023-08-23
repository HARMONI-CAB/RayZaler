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

  connectAll();

}

void
SessionTabWidget::connectAll()
{
  connect(
        m_session,
        SIGNAL(modelChanged()),
        this,
        SLOT(onModelChanged()));
}

SessionTabWidget::~SessionTabWidget()
{
  delete ui;
}

SimulationSession *
SessionTabWidget::session() const
{
  return m_session;
}

void
SessionTabWidget::updateModel()
{
  m_glWidget->update();
}

void
SessionTabWidget::onModelChanged()
{
  updateModel();
}
