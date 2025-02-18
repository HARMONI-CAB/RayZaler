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
#include <MediumBoundaries/ConicLens.h>
#include <Surfaces/Conic.h>

using namespace RZ;

ConicLensBoundary::ConicLensBoundary()
{
  setSurfaceShape(new ConicSurface(0.5, 1, 0));
  setEMInterface(new DielectricEMInterface);
}

std::string
ConicLensBoundary::name() const
{
  return "ConicLensBoundary";
}

void
ConicLensBoundary::setRadius(Real R)
{
  surfaceShape<ConicSurface>()->setRadius(R);
}

void
ConicLensBoundary::setCurvatureRadius(Real Rc)
{
  surfaceShape<ConicSurface>()->setCurvatureRadius(Rc);
}


void
ConicLensBoundary::setConicConstant(Real K)
{
  surfaceShape<ConicSurface>()->setConicConstant(K);
}

void
ConicLensBoundary::setCenterOffset(Real x, Real y)
{
  surfaceShape<ConicSurface>()->setCenterOffset(x, y);
}

void
ConicLensBoundary::setRefractiveIndex(Real in, Real out)
{
  emInterface<DielectricEMInterface>()->setRefractiveIndex(in, out);
}

void
ConicLensBoundary::setConvex(bool convex)
{
  if (convex != m_convex) {
    m_convex = convex;
    surfaceShape<ConicSurface>()->setConvex(convex);
  }
}
