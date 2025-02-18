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

#include <EMInterface.h>
#include <RayTracingEngine.h>

using namespace RZ;

EMInterface::~EMInterface()
{

}

void
EMInterface::setTransmission(Real tx)
{
  m_transmission     = tx;
  m_txMap            = nullptr;
  m_fullyOpaque      = false;
  m_fullyTransparent = false;

  if (tx <= 0) {
    m_fullyOpaque      = true;
    m_transmission     = 0;
  } else if (tx >= 1.) {
    m_fullyTransparent = true;
    m_transmission     = 1.;
  }
}

void
EMInterface::setTransmission(
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
    m_txMap = &map;
  else
    m_txMap = nullptr;
}

void
EMInterface::blockLight(RayBeam &beam)
{
  uint64_t i;
  uint64_t count  = beam.count;
  auto &state     = randState();

  // Block light by means of transmission map
  if (m_txMap != nullptr) {
    std::vector<Real> const &map = *m_txMap;

    for (i = 0; i < count; ++i) {
      if (beam.isIntercepted(i)) { 
        Real coordX = beam.destinations[3 * i + 0];
        Real coordY = beam.destinations[3 * i + 1];

        int  pixI   = +floor(coordX / m_hx) + m_cols / 2;
        int  pixJ   = -floor(coordY / m_hy) + m_rows / 2;

        if (pixI >= 0 && pixI < m_cols && pixJ >= 0 && pixJ < m_rows)
          if (map[pixI + pixJ * m_stride] < state.randu())
            beam.prune(i);
      }
    }
  } else {
    if (!m_fullyTransparent) {
      if (m_fullyOpaque) {
        // Fully opaque. Block all intercepted rays unconditionally
        for (i = 0; i < count; ++i)
          if (beam.isIntercepted(i))
            beam.prune(i);
      } else {
        // Partially opaque. Block rays according to its transmission probability.
        Real tx = m_transmission;
        for (i = 0; i < count; ++i)
          if (beam.isIntercepted(i))
            if (tx < state.randu())
              beam.prune(i);
      }
    }
  }
}
