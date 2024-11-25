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

#ifndef _RZ_RANDOM_H
#define _RZ_RANDOM_H

#include <Vector.h>
#include <random>

#define RZ_SHARED_STATE_DEFAULT_SEED 0x12345

namespace RZ {
  class ExprRandomState {
    uint64_t                             m_epoch = 0;
    std::mt19937_64                      m_generator;
    std::uniform_real_distribution<Real> m_uniform;
    std::normal_distribution<Real>       m_normal;
  public:
    ExprRandomState(uint64_t seed = RZ_SHARED_STATE_DEFAULT_SEED);

    void update();
    void setSeed(uint64_t seed);
    uint64_t epoch() const;
    Real randu();
    Real randn();
  };
}

#endif // _RZ_RANDOM_H
