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

#include <Zernike.h>

using namespace RZ;

static int
factorial(int n)
{ 
  return n < 2  ? 1 : n * factorial(n - 1); 
}

void
Zernike::initFromNL(unsigned int n, int l)
{
  m_n = n;
  m_l = l;
  m_m = l >= 0 ? l : -l;
  m_nRadTerm = 1 + ((m_n - m_m) / 2);
  m_coefs.resize(m_nRadTerm);
  m_powers.resize(m_nRadTerm);

  int sign = -1;

  for (unsigned i = 0; i < m_nRadTerm; ++i) {
    sign *= -1;

    m_powers[i] = m_n - (i << 1);
    m_coefs[i]  = sign * factorial(m_n - i) / (
        factorial(i) 
      * factorial((m_n + m_m) / 2 - i)
      * factorial((m_n - m_m) / 2 - i));
  }
}

Zernike::Zernike(unsigned int j)
{
  int n = ceil(.5 * (-3 + sqrt(9 + 8 * j)));
  int l = 2 * j - n * (n + 2);

  initFromNL(n, l);
}

Zernike::Zernike(unsigned int n, int l)
{
  initFromNL(n, l);
}

Real
Zernike::operator()(double x, double y) const
{
  Real rho = sqrt(x * x + y * y);
  Real phi = atan2(y, x);
  Real v   = 0;

  for (unsigned i = 0; i < m_nRadTerm; ++i)
    v += m_coefs[i] * pow(rho, m_powers[i]);
  
  if (m_l > 0)
    v *= cos(m_l * phi);
  else if (m_l < 0)
    v *= sin(-m_l * phi);

  return v;
}

Real
Zernike::gradX(double x, double y) const
{
  Real rho2   = x * x + y * y;
  Real rho    = sqrt(rho2);
  Real drhodx = x / rho;
  Real phi    = atan2(y, x);
  Real dphidx = - y / rho2;
  Real v      = 0;
  Real dvdx   = 0;

  for (unsigned i = 0; i < m_nRadTerm; ++i) {
    v    += m_coefs[i] * pow(rho, m_powers[i]);
    if (m_powers[i] != 0)
      dvdx += m_coefs[i] * (m_powers[i] * pow(rho, m_powers[i] - 1)) * drhodx;
  }
  
  // gradX = cos (...) * dv/dx + d(cos(...))/dx * v
  if (m_l > 0)
    dvdx = cos(m_l * phi) * dvdx  - sin(m_l * phi) * m_l * dphidx * v;
  else if (m_l < 0)
    dvdx = sin(-m_l * phi) * dvdx - cos(m_l * phi) * m_l * dphidx * v;

  return dvdx;
}

Real
Zernike::gradY(double x, double y) const
{
  Real rho2   = x * x + y * y;
  Real rho    = sqrt(rho2);
  Real drhody = y / rho;
  Real phi    = atan2(y, x);
  Real dphidy = x / rho2;
  Real v      = 0;
  Real dvdy   = 0;

  for (unsigned i = 0; i < m_nRadTerm; ++i) {
    v    += m_coefs[i] * pow(rho, m_powers[i]);
    if (m_powers[i] != 0)
      dvdy += m_coefs[i] * (m_powers[i] * pow(rho, m_powers[i] - 1)) * drhody;
  }
  
  // gradY = cos (...) * dv/dy + d(cos(...))/dy * v
  if (m_l > 0)
    dvdy = cos(m_l * phi) * dvdy  - sin(m_l * phi) * m_l * dphidy * v;
  else if (m_l < 0)
    dvdy = sin(-m_l * phi) * dvdy - cos(m_l * phi) * m_l * dphidy * v;

  return dvdy;
}
