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
