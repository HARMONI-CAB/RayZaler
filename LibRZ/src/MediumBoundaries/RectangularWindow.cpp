//
//  Copyright (c) 2025 Gonzalo José Carracedo Carballal
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
#include <MediumBoundaries/RectangularWindow.h>
#include <Surfaces/Rectangular.h>

using namespace RZ;

RectangularWindowBoundary::RectangularWindowBoundary()
{
  setSurfaceShape(new RectangularFlatSurface());
  setEMInterface(new DielectricEMInterface);
  emInterface<DielectricEMInterface>()->setSurroundingMedium(nullptr);
  setReversible(true);
}

std::string
RectangularWindowBoundary::name() const
{
  return "RectangularWindowBoundary";
}

void
RectangularWindowBoundary::setWidth(Real width)
{
  surfaceShape<RectangularFlatSurface>()->setWidth(width);
}

void
RectangularWindowBoundary::setHeight(Real height)
{
  surfaceShape<RectangularFlatSurface>()->setHeight(height);
}

void
RectangularWindowBoundary::setMedia(const EMMedium *positive, const EMMedium *negative)
{
  emInterface<DielectricEMInterface>()->setMedia(positive, negative);
}
