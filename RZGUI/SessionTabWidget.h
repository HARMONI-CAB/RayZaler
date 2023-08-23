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

  void connectAll();

public:
  explicit SessionTabWidget(SimulationSession *, QWidget *parent = nullptr);
  ~SessionTabWidget() override;

  SimulationSession *session() const;
  void updateModel();

private:
  Ui::SessionTabWidget *ui;

public slots:
  void onModelChanged();

};

#endif // SESSIONTABWIDGET_H
