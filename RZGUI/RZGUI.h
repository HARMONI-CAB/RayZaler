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

#ifndef RZGUI_H
#define RZGUI_H

#define RZ_ORGANIZATION_NAME   "Actinid"
#define RZ_ORGANIZATION_DOMAIN "actinid.org"
#define RZ_APPLICATION_NAME    "RayZaler"

#include <QSettings>
#include "ColorSettings.h"

class RZGUISingleton {
  QSettings *m_settings = nullptr;
  static RZGUISingleton *m_instance;

  RZGUISingleton();

public:
  static RZGUISingleton *instance();

  template<class T> static bool
  loadSetting(T &dest, const char *name)
  {
    QSettings *settings = instance()->settings();
    if (settings->contains(name)
        && settings->value(name).canConvert(QMetaType::fromType<T>())) {
      dest = settings->value(name).value<T>();
      return true;
    }

    return false;
  }


  template<class T> static void
  saveSetting(T const &orig, const char *name)
  {
    QSettings *settings = instance()->settings();
    settings->setValue(name, QVariant::fromValue(orig));
  }

  static void
  sync()
  {
    QSettings *settings = instance()->settings();
    settings->sync();
  }

  QSettings *settings() const;

  static inline bool
  loadColorSettings(ColorSettings &settings) {
    return loadSetting(settings, "Colors");
  }

  static inline void
  saveColorSettings(ColorSettings const &settings) {
    saveSetting(settings, "Colors");
  }
};

#endif // RZGUI_H
