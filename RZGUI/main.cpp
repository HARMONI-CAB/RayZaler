#include "MainWindow.h"
#include <QApplication>
#include <GL/glut.h>
#include <Singleton.h>
#include <RZGUI.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;

  w.show();
  return a.exec();
}
