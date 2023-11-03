#ifndef _ZERNIKE_H
#define _ZERNIKE_H

#include <Vector.h>
#include <vector>

//
// Implementation inspired from https://www.mrao.cam.ac.uk/~bn204/oof/zernikes.html
//
namespace RZ {
  class Zernike {
      unsigned int m_n = 0; // Radial order
      int          m_l = 0; // Angular order

      int          m_m;
      int          m_nRadTerm;

      std::vector<Real> m_coefs;
      std::vector<int>  m_powers;

      void initFromNL(unsigned, int);

    public:
      inline unsigned
      n() const
      {
        return m_n;
      }

      inline int
      m() const
      {
        return m_m;
      }

      inline int
      l() const
      {
        return m_l;
      }

      Zernike(unsigned int n, int l);
      Zernike(unsigned int j = 0);
      Real operator()(double x, double y) const;

      Real gradX(double x, double y) const;
      Real gradY(double x, double y) const;
  };
}

#endif // _ZERNIKE_H
