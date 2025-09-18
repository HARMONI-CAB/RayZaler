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
#include <MediumBoundaries/ConicDoublet.h>
#include <Surfaces/Conic.h>

using namespace RZ;

ConicDoubletBoundary::ConicDoubletBoundary()
{
  setSurfaceShape(new ConicSurface(0.5, 1, 0));
  setEMInterface(new DielectricEMInterface);
}

std::string
ConicDoubletBoundary::name() const
{
  return "ConicLensBoundary";
}

void
ConicDoubletBoundary::setRadius(Real R)
{
  surfaceShape<ConicSurface>()->setRadius(R);
}

void
ConicDoubletBoundary::setCurvatureRadius(Real Rc)
{
  surfaceShape<ConicSurface>()->setCurvatureRadius(Rc);
}


void
ConicDoubletBoundary::setConicConstant(Real K)
{
  surfaceShape<ConicSurface>()->setConicConstant(K);
}

void
ConicDoubletBoundary::setCenterOffset(Real x, Real y)
{
  surfaceShape<ConicSurface>()->setCenterOffset(x, y);
}

void
ConicDoubletBoundary::setRefractiveIndex(Real in, Real out)
{
  emInterface<DielectricEMInterface>()->setRefractiveIndex(in, out);
}

void
ConicDoubletBoundary::setConvex(bool convex)
{
  if (convex != m_convex) {
    m_convex = convex;
    surfaceShape<ConicSurface>()->setConvex(convex);
  }
}

