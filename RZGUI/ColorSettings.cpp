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

#include "ColorSettings.h"

bool
ColorSettings::operator==(const ColorSettings &settings)
{
  bool equals = true;

  equals = equals && settings.bgAbove == bgAbove;
  equals = equals && settings.bgBelow == bgBelow;
  equals = equals && settings.path    == path;
  equals = equals && settings.grid    == grid;

  return equals;
}

QDataStream &
operator<< (QDataStream &out, const ColorSettings &rhs)
{
  out << rhs.bgAbove;
  out << rhs.bgBelow;
  out << rhs.path;
  out << rhs.grid;
  return out;
}

//  undefined reference to `operator>>(QDataStream&, ColorSettings const&)
QDataStream &
operator>> (QDataStream &in, ColorSettings &rhs)
{
  in >> rhs.bgAbove;
  in >> rhs.bgBelow;
  in >> rhs.path;
  in >> rhs.grid;
  return in;
}
