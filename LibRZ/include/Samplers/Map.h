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

#ifndef _SAMPLERS_MAP_H
#define _SAMPLERS_MAP_H

#include <Samplers/Sampler.h>
#include <random>

namespace RZ {
  class MapSampler : public Sampler {
      std::default_random_engine m_generator;
      Real m_width    = 1.;
      Real m_pxToUnit = 1.;
      
      unsigned m_stride = 1;
      
      std::vector<Real> m_lambda;
      unsigned int m_cols = 1, m_rows = 1;

      void normalize();

    protected:
      virtual bool sampleRandom(std::vector<Vec3> &);
      virtual bool sampleUniform(std::vector<Vec3> &);

    public:
      MapSampler();
      MapSampler(std::string const &);
      
      virtual void setRadius(Real R) override;
      void setMap(
        std::vector<Real> const &map,
        unsigned int width,
        unsigned int stride = 0);
      void setFromPNG(std::string const &);
  };
}

#endif // _SAMPLERS_PNG_H
