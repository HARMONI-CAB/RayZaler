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

#include <Samplers/Point.h>
#include <Logger.h>

using namespace RZ;

bool
PointSampler::sampleRandom(std::vector<Vec3> &dest)
{
  return sampleUniform(dest);
}

bool
PointSampler::sampleUniform(std::vector<Vec3> &dest)
{
  unsigned int N = dest.size();

  if (N == 0) {
    RZError("Cannot sample a sequence of zero vectors\n");
    return false;
  }

  for (auto j = 0; j < N; ++j)
    dest[j] = Vec3::zero();

  return true;
}

void
PointSampler::setRadius(Real)
{
  // Do nothing
}
