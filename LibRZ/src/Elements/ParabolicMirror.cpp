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

#include <Elements/ParabolicMirror.h>

using namespace RZ;

RZ_DESCRIBE_ELEMENT_FROM(ParabolicMirror, ConicMirror, "Mirror with parabolic surface")
{
  hiddenProperty("conic", -1, "Conic constant of the mirror (overriden)");
}

bool
ParabolicMirror::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "conic")
    return false;
  
  return ConicMirror::propertyChanged(name, value);
}

ParabolicMirror::ParabolicMirror(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : ConicMirror(factory, name, frame, parent)
{
  ConicMirror::propertyChanged("conic", -1.);
}

ParabolicMirror::~ParabolicMirror()
{
}
