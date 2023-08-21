#include "MainWindow.h"
#include <QApplication>
#include <GL/glut.h>
#include <Singleton.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;

  RZ::RZInit();

  glutInit(&argc, argv);

  w.show();
  return a.exec();
}
