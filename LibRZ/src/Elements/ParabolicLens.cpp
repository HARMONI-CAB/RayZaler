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

#include <Elements/ParabolicLens.h>

using namespace RZ;

bool
ParabolicLens::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "conic" || name == "frontConic" || name == "backConic")
    return false;

  return ConicLens::propertyChanged(name, value);
}

ParabolicLens::ParabolicLens(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : ConicLens(factory, name, frame, parent)
{
  ConicLens::propertyChanged("conic", -1.);
}

ParabolicLens::~ParabolicLens()
{
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
ParabolicLensFactory::name() const
{
  return "ParabolicLens";
}

Element *
ParabolicLensFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new ParabolicLens(this, name, pFrame, parent);
}
