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

#include <Samplers/Circular.h>
#include <Logger.h>

using namespace RZ;

bool
CircularSampler::sampleRandom(std::vector<Vec3> &dest)
{
  unsigned int N = dest.size();

  if (N == 0) {
    RZError("Cannot sample a sequence of zero vectors\n");
    return false;
  }

  for (auto j = 0; j < N; ++j) {
    Real sep       = m_R * sqrt(.5 * (1 + RZ_URANDSIGN));
    Real angle     = RZ_URANDSIGN * M_PI;
    dest[j].x      = sep * cos(angle);
    dest[j].y      = sep * sin(angle);
    dest[j].z      = 0;
  }

  return true;
}

bool
CircularSampler::sampleUniform(std::vector<Vec3> &dest)
{
  unsigned int N = dest.size();

  if (N == 0) {
    RZError("Cannot sample a sequence of zero vectors\n");
    return false;
  }

  Real nR2 = 1;
  Real nR  = 1;

  Real dA = (M_PI * nR2) / N;
  Real dh = sqrt(dA);
  Real x0 = -dh * floor(2 * nR / dh) / 2;
  Real y0 = -dh * (floor(sqrt((nR2 - x0 * x0)) / dh * 2) / 2);
  unsigned int i = 0, j = 0;

  for (;;) {
    Real y = y0 + j++ * dh;

    if (y > fabs(y0)) {
      x0 += dh;
      if (x0 > nR)
        break;
      
      y0  = -dh * (floor(sqrt((nR2 - x0 * x0)) / dh - .5) + .5);
      y   = y0;
      j   = 0;
    }

    if (i == dest.size())
      dest.resize(i + 1);
    
    dest[i].x = x0 * m_R;
    dest[i].y = y  * m_R;
    dest[i].z = 0;

    ++i;
  }

  return true;
}

void
CircularSampler::setRadius(Real R)
{
  m_R  = R;
  m_R2 = R * R;
  reset();
}
