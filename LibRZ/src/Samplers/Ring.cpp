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

#include <Samplers/Ring.h>

using namespace RZ;

bool
RingSampler::sampleRandom(std::vector<Vec3> &dest)
{
  unsigned int N = dest.size();

  if (N == 0)
    return false;

  for (auto j = 0; j < N; ++j) {
    Real angle     = RZ_URANDSIGN * M_PI;
    dest[j].x      = m_R * cos(angle);
    dest[j].y      = m_R * sin(angle);
    dest[j].z      = 0;
  }

  return true;
}

bool
RingSampler::sampleUniform(std::vector<Vec3> &dest)
{
  unsigned int N = dest.size();

  if (N == 0)
    return false;

  Real dTheta = 2 * M_PI / N;

  for (auto j = 0; j < N; ++j) {
    Real theta = j * dTheta;

    dest[j].x = m_R * cos(theta);
    dest[j].y = m_R * sin(theta);
    dest[j].z = 0;
  }

  return true;
}

void
RingSampler::setRadius(Real R)
{
  m_R  = R;
  reset();
}
