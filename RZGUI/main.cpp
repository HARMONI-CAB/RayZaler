//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

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
  bool refersToSimConfig = false;
  const char *simCfg = nullptr;

  loadFonts();

  w.show();

  for (auto i = 1; i < argc; ++i) {
    if (argv[i][0] != '-') {
      if (refersToSimConfig) {
        simCfg = argv[i];
        refersToSimConfig = false;
      } else {
        w.pushDelayedOpenFile(argv[i], simCfg);
        simCfg = nullptr;
      }
    } else {
      std::string opt = argv[i];

      if (opt == "-s" || opt == "--simconfig")
        refersToSimConfig = true;
    }
  }

  w.notifyReady();
  
  return a.exec();
}
