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

#include <EMInterfaces/DielectricEMInterface.h>
#include <MediumBoundaries/CircularWindow.h>
#include <Surfaces/Circular.h>

using namespace RZ;

CircularWindowBoundary::CircularWindowBoundary()
{
  setSurfaceShape(new CircularFlatSurface(.5));
  setEMInterface(new DielectricEMInterface);
}

std::string
CircularWindowBoundary::name() const
{
  return "CircularWindowBoundary";
}

void
CircularWindowBoundary::setRadius(Real R)
{
  surfaceShape<CircularFlatSurface>()->setRadius(R);
}

void
CircularWindowBoundary::setRefractiveIndex(Real in, Real out)
{
  emInterface<DielectricEMInterface>()->setRefractiveIndex(in, out);
}
