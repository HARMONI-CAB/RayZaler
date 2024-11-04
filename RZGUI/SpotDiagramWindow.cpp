#include "SpotDiagramWindow.h"
#include "ui_SpotDiagramWindow.h"
#include "ScatterWidget.h"

SpotDiagramWindow::SpotDiagramWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::SpotDiagramWindow)
{
  ui->setupUi(this);

  setCentralWidget(new ScatterWidget());
}

SpotDiagramWindow::~SpotDiagramWindow()
{
  delete ui;
}
