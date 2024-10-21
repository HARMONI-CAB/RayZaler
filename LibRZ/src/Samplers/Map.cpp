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

#include <Samplers/Map.h>
#include "Helpers.h"
#include <png++/png.hpp>
#include <sys/time.h>

using namespace RZ;

void
MapSampler::normalize()
{
  RZ::Real sum = sumPrecise<RZ::Real>(m_lambda.data(), m_lambda.size());
  for (auto &p : m_lambda)
    p /= sum;
}

bool
MapSampler::sampleRandom(std::vector<Vec3> &dest)
{
  return sampleUniform(dest);
}

bool
MapSampler::sampleUniform(std::vector<Vec3> &dest)
{
  unsigned int N = dest.size();
  unsigned int points;
  unsigned int count = 0;
  unsigned int incAmount = 1;
  Real m_left, m_top;

  
  std::uniform_real_distribution uniform(0., 1.);

  if (N == 0)
    return false;

  m_left  = -m_width / 2;
  m_top   = -m_pxToUnit * m_rows / 2;

  for (unsigned j = 0; j < m_rows; ++j) {
    for (unsigned i = 0; i < m_cols; ++i) {
      std::poisson_distribution<unsigned int> poisson(N * m_lambda[j * m_stride + i]);
      points = poisson(m_generator);

      for (unsigned n = 0; n < points; ++n) {
        dest[count].x = m_pxToUnit * (i + uniform(m_generator)) + m_left;
        dest[count].y = m_pxToUnit * (j + uniform(m_generator)) + m_top;
        dest[count].z = 0;

        ++count;

        if (count == dest.size()) {
          dest.resize(dest.size() + incAmount);
          incAmount *= 2;
        }
      }
    }
  }

  dest.resize(count);

  return true;
}

void
MapSampler::setRadius(Real R)
{
  m_width  = 2 * R;
  m_pxToUnit = m_width / m_cols;
  reset();
}

void
MapSampler::setMap(
  std::vector<Real> const &map,
  unsigned int cols,
  unsigned int stride)
{
  unsigned int rows;
  size_t allocSize;
  size_t mapSize;
  size_t zeroSize;

  mapSize  = map.size();

  if (stride == 0)
    stride = cols;

  rows = (mapSize + stride - 1) / stride;

  allocSize = rows * stride;
  m_lambda.resize(allocSize);

  zeroSize = allocSize - mapSize;
  memcpy(m_lambda.data(), map.data(), mapSize * sizeof(Real));

  if (zeroSize > 0)
    memset(m_lambda.data() + mapSize, 0, zeroSize * sizeof(Real));

  m_cols   = cols;
  m_rows   = rows;
  m_stride = stride;

  normalize();
}

void
MapSampler::setFromPNG(std::string const &path)
{
  png::image<png::gray_pixel_16> inputMap(path);
  size_t allocSize;
  unsigned int i, j;
  unsigned int stride = inputMap.get_width();
  unsigned int rows, cols;

  cols = stride;
  rows = inputMap.get_height();

  allocSize = cols * rows;
  m_lambda.resize(allocSize);

  for (i = 0; i < rows; ++i) {
    for (j = 0; j < cols; ++j) {
      auto pixel = inputMap.get_pixel(j, i);
      m_lambda[j + i * stride] = pixel / 65535.;
    }
  }

  m_rows   = rows;
  m_cols   = cols;
  m_stride = stride;

  normalize();
}

MapSampler::MapSampler()
{
  struct timeval tv;

  gettimeofday(&tv, nullptr);

  m_generator.seed(tv.tv_sec + tv.tv_usec);
}

MapSampler::MapSampler(std::string const &path) : MapSampler()
{
  setFromPNG(path);
}
