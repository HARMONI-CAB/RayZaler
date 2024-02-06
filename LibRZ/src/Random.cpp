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

