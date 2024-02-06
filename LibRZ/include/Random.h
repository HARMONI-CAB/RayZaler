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
