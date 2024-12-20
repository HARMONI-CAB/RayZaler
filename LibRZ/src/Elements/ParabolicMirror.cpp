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

///////////////////////////////// Factory //////////////////////////////////////
std::string
ParabolicMirrorFactory::name() const
{
  return "ParabolicMirror";
}

Element *
ParabolicMirrorFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new ParabolicMirror(this, name, pFrame, parent);
}
