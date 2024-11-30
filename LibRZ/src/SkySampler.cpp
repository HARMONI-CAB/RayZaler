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

#include <SkySampler.h>

using namespace RZ;

SkySampler::SkySampler(Vec3 const &direction)
{
  m_centralAxis = direction;
}

void
SkySampler::setShape(SkyObjectShape shape)
{
  m_shape = shape;
  m_dirty = true;
}

void
SkySampler::setNumRays(unsigned rays)
{
  m_nRays = rays;
  m_dirty = true;
}

void
SkySampler::setDiameter(Real diameter)
{
  m_diameter = diameter;
  m_dirty    = true;
}

void
SkySampler::setRandom(bool random)
{
  m_random = random;
  m_dirty  = true;
}

void
SkySampler::setPath(std::string const &path)
{
  m_path  = path;
  m_dirty = true;
}

void
SkySampler::reconfigure()
{
  if (m_shape == CircleLike) {
    m_sampler = &m_circularSampler;
  } else  if (m_shape == RingLike) {
    m_sampler = &m_ringSampler;
  } else if (m_shape == Extended) {
    m_sampler = &m_mapSampler;
    m_mapSampler.setFromPNG(m_path);
  } else {
    m_sampler = nullptr;
  }
  
  if (m_sampler != nullptr) {
    m_sampler->setRandom(m_random);
    m_sampler->setRadius(m_diameter / 2);
    m_sampler->sample(m_nRays);
  }

  m_dirty = false;
}

bool
SkySampler::get(Vec3 &output)
{
  if (m_dirty)
    reconfigure();

  if (m_sampler == nullptr) {
    output = m_centralAxis;
  } else {
    Vec3 samp;
    if (!m_sampler->get(samp))
      return false;
    
    output = Matrix3::rot(Vec3::eY(), -samp.x) * Matrix3::rot(Vec3::eX(), samp.y) * m_centralAxis;
  }

  return true;
}
