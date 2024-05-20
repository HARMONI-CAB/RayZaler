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

#include <Random.h>

using namespace RZ;
#define MERSENNE_TWISTER_DISCARD 100

ExprRandomState::ExprRandomState(uint64_t seed)
{
  setSeed(seed);
}

void
ExprRandomState::update()
{
  ++m_epoch;
}

void
ExprRandomState::setSeed(uint64_t seed)
{
  m_generator.seed(static_cast<int>(seed));
  m_generator.discard(MERSENNE_TWISTER_DISCARD);
  m_uniform.reset();
  m_normal.reset();

  update();
}

uint64_t
ExprRandomState::epoch() const
{
  return m_epoch;
}

Real
ExprRandomState::randu()
{
  return m_uniform(m_generator);
}

Real
ExprRandomState::randn()
{
  return m_normal(m_generator);
}

