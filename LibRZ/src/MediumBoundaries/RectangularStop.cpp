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

#include <MediumBoundaries/RectangularStop.h>
#include <ReferenceFrame.h>
#include <Surfaces/Rectangular.h>

using namespace RZ;

RectangularStopBoundary::RectangularStopBoundary()
{
  setSurfaceShape(new RectangularFlatSurface());

  surfaceShape<RectangularFlatSurface>()->setHeight(m_height);
  surfaceShape<RectangularFlatSurface>()->setWidth(m_width);
}

std::string
RectangularStopBoundary::name() const
{
  return "RectangularStopBoundary";
}

void
RectangularStopBoundary::setWidth(Real width)
{
  m_width = width;
  surfaceShape<RectangularFlatSurface>()->setWidth(m_width);
}

void
RectangularStopBoundary::setHeight(Real height)
{
  m_height = height;
  surfaceShape<RectangularFlatSurface>()->setHeight(m_height);
}

void
RectangularStopBoundary::transfer(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Vec3 center  = plane->getCenter();
  Vec3 tX      = plane->eX();
  Vec3 tY      = plane->eY();
  Real halfW   = .5 * m_width;
  Real halfH   = .5 * m_height;

  for (i = 0; i < count; ++i) {
    Vec3 coord  = Vec3(beam.destinations + 3 * i) - plane->getCenter();
    Real coordX = coord * tX;
    Real coordY = coord * tY;

    if (fabs(coordX) >= halfW || fabs(coordY) >= halfH)
      beam.prune(i);
  }
}
