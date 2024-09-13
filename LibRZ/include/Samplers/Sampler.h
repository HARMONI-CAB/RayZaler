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

#ifndef _SAMPLERS_SAMPLER_H
#define _SAMPLERS_SAMPLER_H

#include <Vector.h>
#include <Matrix.h>
#include <vector>

namespace RZ {
  class Sampler {
    bool              m_random = false;
    std::vector<Vec3> m_samples;
    unsigned int      m_ptr = 0;

    bool ensureSamples(unsigned int N);

  protected:
    virtual bool sampleRandom(std::vector<Vec3> &) = 0;
    virtual bool sampleUniform(std::vector<Vec3> &) = 0;

  public:
    virtual void setRadius(Real) = 0;
    void setRandom(bool);
    void reset();

    bool sample(std::vector<Vec3> &dest);
    bool sample(std::vector<Vec3> &dest, Matrix3 const &sys, Vec3 const &center);

    bool sample(unsigned int N);
    bool get(Vec3 &dest);
    bool get(Vec3 &dest, Matrix3 const &sys, Vec3 const &center);
    Vec3 &get();
  };
}

#endif // _SAMPLERS_SAMPLER_H
