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

#ifndef _EM_INTERFACES_PARAXIAL_ZERNIKE_H
#define _EM_INTERFACES_PARAXIAL_ZERNIKE_H

#include <EMInterface.h>
#include <Zernike.h>

namespace RZ {
  class ParaxialZernikeEMInterface : public EMInterface {
      Real m_radius        = .5;
      std::vector<Zernike> m_poly;
      std::vector<Real>    m_coef;
      std::vector<bool>    m_nonz;
      int                  m_firstNz = -1;
      int                  m_lastNz  = -1;
      
      Real m_muOut         = 1.5;
      Real m_muIn          = 1;
      Real m_IOratio       = 1 / 1.5;

      Real dZdx(Real x, Real y) const;
      Real dZdy(Real x, Real y) const;

    public:
      inline Real
      coef(unsigned int ansi) const
      {
        if (ansi >= m_coef.size())
          return 0;
          
        return m_coef[ansi];
      }

      Real Z(Real x, Real y) const;
      void setRadius(Real);
      void setCoef(unsigned int ansi, Real value);
      void setRefractiveIndex(Real, Real);

      virtual std::string name() const override;
      virtual void transmit(RayBeamSlice const &beam) override;
      virtual ~ParaxialZernikeEMInterface() override;
  };
}

#endif // _EM_INTERFACES_PARAXIAL_ZERNIKE_H

