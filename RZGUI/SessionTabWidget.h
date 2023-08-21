#ifndef SESSIONTABWIDGET_H
#define SESSIONTABWIDGET_H

#include <QWidget>

namespace Ui {
  class SessionTabWidget;
}

class SimulationSession;
class RZGUIGLWidget;

class SessionTabWidget : public QWidget
{
  Q_OBJECT

  SimulationSession *m_session;  // Borrowed
  RZGUIGLWidget     *m_glWidget; // Borrowed

public:
  explicit SessionTabWidget(SimulationSession *, QWidget *parent = nullptr);
  ~SessionTabWidget();

private:
  Ui::SessionTabWidget *ui;
};

#endif // SESSIONTABWIDGET_H
