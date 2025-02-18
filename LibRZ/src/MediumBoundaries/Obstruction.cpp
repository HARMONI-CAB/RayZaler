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

#include <EMInterfaces/DummyEMInterface.h>
#include <MediumBoundaries/Obstruction.h>
#include <Surfaces/Circular.h>

using namespace RZ;

ObstructionBoundary::ObstructionBoundary()
{
  setReversible(true);

  setSurfaceShape(new CircularFlatSurface(.5));
  surfaceShape<CircularFlatSurface>()->setObstruction(true);

  setEMInterface(new DummyEMInterface);
}

std::string
ObstructionBoundary::name() const
{
  return "ObstructionBoundary";
}

void
ObstructionBoundary::setRadius(Real R)
{
  surfaceShape<CircularFlatSurface>()->setRadius(R);
  emInterface<DummyEMInterface>()->setTransmission(0);
}

void
ObstructionBoundary::setObstructionMap(
  Real width,
  Real height,
  std::vector<Real> const &map,
  unsigned int cols,
  unsigned int rows,
  unsigned int stride)
{
  Real R = sqrt(.25 * (width * width + height * height));

  surfaceShape<CircularFlatSurface>()->setRadius(R);
  emInterface<DummyEMInterface>()->setTransmission(
    width,
    height,
    map,
    cols,
    rows,
    stride);
}
