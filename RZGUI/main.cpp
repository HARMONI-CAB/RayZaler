#include "MainWindow.h"
#include <QApplication>
#include <GL/glut.h>
#include <Singleton.h>
#include <RZGUI.h>
#include <FT2Facade.h>
#include <QResource>
#include <Logger.h>

bool
loadEmbeddedFontAs(const char *name, const char *file)
{
  QResource resource("/fonts/fonts/" + QString(file));
  RZ::FT2Facade *ft = RZ::Singleton::instance()->freetype();
  FT_Error error;

  if (!resource.isValid()) {
    RZError("Cannot load embedded font file `%s': resource not found\n", file);
    return false;
  }

  if (!ft->loadFace(name, resource.data(), resource.size(), error)) {
    RZError("Cannot load font `%s': %s: %s\n", name, file, FT_Error_String(error));
    return false;
  }

  return true;
}

void
loadFonts()
{
  loadEmbeddedFontAs("gridfont", "LTSuperior-Regular.otf");
  loadEmbeddedFontAs("gridfont-medium", "LTSuperior-Medium.otf");
  loadEmbeddedFontAs("gridfont-bold", "LTSuperior-Bold.otf");
  loadEmbeddedFontAs("gridfont-semibold", "LTSuperior-Semibold.otf");
}

int
main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;

  loadFonts();

  w.show();

  for (auto i = 1; i < argc; ++i)
    if (argv[i][0] != '-')
      w.pushDelayedOpenFile(argv[i]);

  w.notifyReady();
  
  return a.exec();
}
