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

#include <MediumBoundaries/Obstruction.h>
#include <ReferenceFrame.h>
#include <Surfaces/Circular.h>

using namespace RZ;

ObstructionBoundary::ObstructionBoundary()
{
  setReversible(true);
  setSurfaceShape(new CircularFlatSurface(m_radius));
  surfaceShape<CircularFlatSurface>()->setObstruction(true);
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
  m_radius = R;
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
  m_hx             = width  / cols;
  m_hy             = height / rows;

  m_cols           = cols;
  m_rows           = rows;
  m_stride         = stride;

  if (cols * rows > 0)
    m_obsMapPtr = &map;
  else
    m_obsMapPtr = nullptr;
}

void
ObstructionBoundary::transfer(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t i;
  uint64_t count  = beam.count;
  Vec3 center     = plane->getCenter();
  Vec3 tX         = plane->eX();
  Vec3 tY         = plane->eY();
  Real Rsq        = m_radius * m_radius;
  auto &state     = randState();

  if (m_obsMapPtr != nullptr) {
    std::vector<Real> const &map = *m_obsMapPtr;

    for (i = 0; i < count; ++i) {
      Vec3 coord  = Vec3(beam.destinations + 3 * i) - plane->getCenter();
      Real coordX = coord * tX;
      Real coordY = coord * tY;

      int  pixI   = +floor(coordX / m_hx) + m_cols / 2;
      int  pixJ   = -floor(coordY / m_hy) + m_rows / 2;

      // Obstruction maps are matrices that describe the probability of a
      // lightray of traversing the surface. White is full probability,
      // black is zero probability. Gray is somewhere in the middle.

      if (pixI >= 0 && pixI < m_cols && pixJ >= 0 && pixJ < m_rows) {
        if (map[pixI + pixJ * m_stride] < state.randu()) {
          beam.prune(i);
          continue;
        }
      }
      
      beam.interceptDone(i);
    }
  } else {
    for (i = 0; i < count; ++i) {
      Vec3 coord  = Vec3(beam.destinations + 3 * i) - plane->getCenter();
      Real coordX = coord * tX;
      Real coordY = coord * tY;

      if (coordX * coordX + coordY * coordY <= Rsq)
        beam.prune(i);
      else
        beam.interceptDone(i);
    }
  }
}