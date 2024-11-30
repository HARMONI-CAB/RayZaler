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

#include <SphericalLens.h>
#include <TranslatedFrame.h>

using namespace RZ;

bool
SphericalLens::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "conic")
    return false;

  return ConicLens::propertyChanged(name, value);
}

SphericalLens::SphericalLens(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : ConicLens(factory, name, frame, parent)
{
  ConicLens::propertyChanged("conic", 0.);
}

SphericalLens::~SphericalLens()
{
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
SphericalLensFactory::name() const
{
  return "SphericalLens";
}

Element *
SphericalLensFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new SphericalLens(this, name, pFrame, parent);
}
