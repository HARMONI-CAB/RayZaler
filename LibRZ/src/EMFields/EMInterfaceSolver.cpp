//
//  Copyright (c) 2025 Gonzalo José Carracedo Carballal
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

#include <EMFields/EMInterfaceSolver.h>
#include <EMFields/EMSolver.h>
#include <EMInterface.h>

#include <EMFields/EMIsoIsoSolver.h>

using namespace RZ;

EMInterfaceSolver::EMInterfaceSolver(const EMMedium *m1, const EMMedium *m2)
{
  m_solver = new EMSolver();

  setMedia(m1, m2);
}

EMInterfaceSolver *
EMInterfaceSolver::make(const EMMedium *m1, const EMMedium *m2)
{
  if (m1->isotropic() && m2->isotropic())
    return new EMIsoIsoSolver(m1, m2);

  return nullptr;
}

void
EMInterfaceSolver::setMedia(const EMMedium *m1, const EMMedium *m2)
{
  m_m1 = m1;
  m_m2 = m2;
  m_solver->setMedia(m1, m2);
}

void
EMInterfaceSolver::setBeam(RayBeamSlice const &slice, RayBeam *splinterBeam)
{
  m_currentSlice = &slice;
  m_splinterBeam = splinterBeam;
  m_mainBeam     = slice.beam;

  if (splinterBeam != nullptr) {
    m_secondaryRays = true;
    m_sCount = secondaryBeamCount();

    if (splinterBeam->count < m_mainBeam->count) {
      splinterBeam->allocate(m_mainBeam->count * m_sCount);
    }

    // There is a buch of properties in each ray that we can just keep here
    for (auto i = 0; i < m_sCount; ++i) {
      uint64_t start = i * m_mainBeam->count;
      slice.copyTo(
        RayBeamSlice(
          splinterBeam,
          start + slice.start,
          start + slice.end));
    }
  } else {
    m_sCount = 0;
  }

  m_calculateFields = m_secondaryRays && m_mainBeam->fields;
}

