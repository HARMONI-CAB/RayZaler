#ifndef EXPORTVIEWDIALOG_H
#define EXPORTVIEWDIALOG_H

#include <QDialog>
#include <Matrix.h>

namespace Ui {
  class ExportViewDialog;
}

class SessionTabWidget;
class RZGUIGLWidget;

class ExportViewDialog : public QDialog
{
  Q_OBJECT

  RZGUIGLWidget     *m_glWidget = nullptr;
  SessionTabWidget  *m_sessionTab = nullptr;
  RZ::Real           m_nominalWidth;
  RZ::Real           m_nominalHeight;

  void               renderAndSave();
  void               connectAll();

public:
  explicit ExportViewDialog(QWidget *parent = nullptr);

  void setSessionTabWidget(SessionTabWidget *);
  ~ExportViewDialog();

private:
  Ui::ExportViewDialog *ui;

public slots:
  void onBrowse();
  void onChangeWidthSpin();
  void onChangeHeightSpin();
  void onResetSame();
  void onSave();
  void onCancel();
};

#endif // EXPORTVIEWDIALOG_H
