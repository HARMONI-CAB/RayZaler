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

#include <Samplers/Sampler.h>

using namespace RZ;

void
Sampler::setRandom(bool random)
{
  if (m_random != random) {
    m_random = random;
    reset();
  }
}

void
Sampler::reset()
{
  m_samples.clear();
  m_ptr = 0;
}

bool
Sampler::ensureSamples(unsigned int N)
{
  bool ret = true;

  if (m_samples.size() != N) {
    reset();

    m_samples.resize(N);

    ret = m_random ? sampleRandom(m_samples) : sampleUniform(m_samples);
  }

  return ret;
}

bool
Sampler::sample(std::vector<Vec3> &dest)
{
  reset();

  return m_random ? sampleRandom(dest) : sampleUniform(dest);
}

bool
Sampler::sample(std::vector<Vec3> &dest, Matrix3 const &sys, Vec3 const &center)
{
  if (!sample(dest))
    return false;

  for (auto &p : dest)
    p = sys * p + center;

  return true;
}

bool
Sampler::sample(unsigned int N)
{
  reset();
  return ensureSamples(N);
}

bool
Sampler::get(Vec3 &dest)
{
  if (m_ptr >= m_samples.size())
    return false;

  dest = m_samples[m_ptr++];
  return true;
}

Vec3 &
Sampler::get()
{
  if (m_ptr >= m_samples.size())
    throw std::runtime_error("Failed to acquire coordinates. Buffer exhausted.");

  return m_samples[m_ptr++];
}

bool
Sampler::get(Vec3 &dest, Matrix3 const &sys, Vec3 const &center)
{
  if (!get(dest))
    return false;

  dest = sys * dest + center;
  return true;
}
