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

#include <EMInterfaces/ParaxialZernikeEMInterface.h>
#include <MediumBoundaries/PhaseScreen.h>
#include <Surfaces/Circular.h>

using namespace RZ;

PhaseScreenBoundary::PhaseScreenBoundary()
{
  setSurfaceShape(new CircularFlatSurface(.5));
  setEMInterface(new ParaxialZernikeEMInterface);
}

std::string
PhaseScreenBoundary::name() const
{
  return "PhaseScreenBoundary";
}

void
PhaseScreenBoundary::setRadius(Real R)
{
  surfaceShape<CircularFlatSurface>()->setRadius(R);
  emInterface<ParaxialZernikeEMInterface>()->setRadius(R);
}

void
PhaseScreenBoundary::setCoef(unsigned int ansi, Real value)
{
  emInterface<ParaxialZernikeEMInterface>()->setCoef(ansi, value);
}

Real
PhaseScreenBoundary::Z(Real x, Real y) const
{
  return emInterface<ParaxialZernikeEMInterface>()->Z(x, y);
}

Real
PhaseScreenBoundary::coef(unsigned int ansi) const
{
  return emInterface<ParaxialZernikeEMInterface>()->coef(ansi);
}

void
PhaseScreenBoundary::setRefractiveIndex(Real in, Real out)
{
  emInterface<ParaxialZernikeEMInterface>()->setRefractiveIndex(in, out);
}
