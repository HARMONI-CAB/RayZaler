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

#include <EMInterfaces/ReflectiveEMInterface.h>
#include <MediumBoundaries/RectangularStop.h>
#include <Surfaces/Rectangular.h>

using namespace RZ;

RectangularStopBoundary::RectangularStopBoundary()
{
  setComplementary(true);

  setSurfaceShape(new RectangularFlatSurface());

  surfaceShape<RectangularFlatSurface>()->setHeight(.1);
  surfaceShape<RectangularFlatSurface>()->setWidth(.1);
  
  setEMInterface(new ReflectiveEMInterface);
  emInterface<ReflectiveEMInterface>()->setTransmission(0);
}

std::string
RectangularStopBoundary::name() const
{
  return "RectangularStopBoundary";
}

void
RectangularStopBoundary::setWidth(Real width)
{
  surfaceShape<RectangularFlatSurface>()->setWidth(width);
}

void
RectangularStopBoundary::setHeight(Real height)
{
  surfaceShape<RectangularFlatSurface>()->setHeight(height);
}
