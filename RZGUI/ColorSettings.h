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

#ifndef COLORSETTINGS_H
#define COLORSETTINGS_H

#include <QColor>

struct ColorSettings {
  QColor bgAbove = QColor(1, 1, 1);
  QColor bgBelow = QColor(0x75, 0x75, 0xe5);
  QColor path    = QColor(255, 0, 255);
  QColor grid    = QColor(255, 255, 255);

  bool operator==(const ColorSettings &);
  friend QDataStream &operator<< (QDataStream &out, const ColorSettings &rhs);
  friend QDataStream &operator>> (QDataStream &out, ColorSettings &rhs);
};

#endif // COLORSETTINGS_H
