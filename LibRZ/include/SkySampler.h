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

#ifndef _SKY_SAMPLER_H
#define _SKY_SAMPLER_H

#include <Samplers/Circular.h>
#include <Samplers/Ring.h>
#include <Samplers/Map.h>

namespace RZ {
  enum SkyObjectShape {
    PointLike,
    CircleLike,
    RingLike,
    Extended
  };

  class SkySampler {
    MapSampler      m_mapSampler;
    CircularSampler m_circularSampler;
    RingSampler     m_ringSampler;
    Sampler        *m_sampler = nullptr;
    SkyObjectShape  m_shape = PointLike;
    Real            m_diameter = M_PI / 6;  // Radians
    Vec3            m_centralAxis = -Vec3::eZ();
    bool            m_random = false;
    unsigned int    m_nRays = 1000;
    std::string     m_path;
    bool            m_dirty = true;

    void reconfigure();

  public:
    SkySampler(Vec3 const &direction);
    void setShape(SkyObjectShape);
    void setNumRays(unsigned);
    void setDiameter(Real);
    void setRandom(bool);
    void setPath(std::string const &);
    bool get(Vec3 &);
  };
}

#endif // _SKY_SAMPLER_H
