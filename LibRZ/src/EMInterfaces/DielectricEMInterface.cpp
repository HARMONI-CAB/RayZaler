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

#include <EMInterfaces/DielectricEMInterface.h>
#include <RayTracingEngine.h>
#include <Logger.h>
#include <EMFields/EMSolver.h>
#include <EMFields/EMInterfaceSolver.h>

using namespace RZ;

std::string
DielectricEMInterface::name() const
{
  return "DielectricEMInterface";
}

void
DielectricEMInterface::transmit(
  RayBeamSlice const &slice,
  RayBeam *splinterBeam)
{
  blockLight(slice); // Prune rays according to transmission

  m_ifaceSolver->setBeam(slice, splinterBeam);
  m_ifaceSolver->transmit();
}

DielectricEMInterface::~DielectricEMInterface()
{

}

void
DielectricEMInterface::setSurroundingMedium(const EMMedium *medium)
{
  auto pOld = pMedium();
  auto nOld = nMedium();

  EMInterface::setSurroundingMedium(medium);
  
  if (pOld != pMedium() || nOld != nMedium())
    setMedia(pMedium(), nMedium());
}

void
DielectricEMInterface::setMedia(
  const EMMedium *positive,
  const EMMedium *negative)
{
  EMInterface::setMedia(positive, negative);

  if (m_ifaceSolver != nullptr) {
    if ((pMedium()->isotropic() != m_ifaceSolver->pMedium()->isotropic())
    || (nMedium()->isotropic() != m_ifaceSolver->nMedium()->isotropic())) {
      delete m_ifaceSolver;
      m_ifaceSolver = nullptr;
    }
  }

  if (m_ifaceSolver == nullptr)
    m_ifaceSolver = EMInterfaceSolver::make(pMedium(), nMedium());
  else
    m_ifaceSolver->setMedia(pMedium(), nMedium());
}
