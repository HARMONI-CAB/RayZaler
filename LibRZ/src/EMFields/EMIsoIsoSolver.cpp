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

#include <EMIsoIsoSolver.h>
#include <EMInterface.h>
#include <EMSolver.h>

using namespace RZ;

uint8_t
EMIsoIsoSolver::secondaryBeamCount() const
{
  return 1; // The only reflected ray
}

void
EMIsoIsoSolver::transmit()
{
  for (uint64_t i = m_currentSlice->start; i < m_currentSlice->end; ++i) {
    if (EMInterface::mustTransmitRay(m_mainBeam, i)) {
      Vec3 ui = Vec3(m_mainBeam->directions + 3 * i);
      Vec3 ki = m_mainBeam->neff[i] * ui;
      
      // Configure incident ray
      m_solver->setIncidentRay(
        ki,
        Vec3(m_mainBeam->normals + 3 * i),
        m_mainBeam->Dx[i],
        m_mainBeam->Dy[i],
        Vec3(m_mainBeam->vDx + 3 * i));

      // Calculate transmitted and reflected rays
      auto breakMask = m_solver->rayBreak();

      if (breakMask & TransmittedOrdinary) {
        m_mainBeam->neff[i]  = m_solver->m2->no;
        m_mainBeam->media[i] = m_solver->m2;
        m_solver->uo2.copyToArray(m_mainBeam->directions + 3 * i);
      } else {
        m_mainBeam->prune(i);
      }

      if (m_secondaryRays & ReflectedOrdinary) {
        m_splinterBeam->neff[i]  = m_solver->m1->no;
        m_splinterBeam->media[i] = m_solver->m1;
        m_solver->uo1.copyToArray(m_splinterBeam->directions + 3 * i);
      } else {
        m_splinterBeam->prune(i);
      }

      // Calculate fields, only for fully broken rays
      if (m_calculateFields) {
        if (breakMask == (TransmittedOrdinary | ReflectedOrdinary)) {
          EMFields reflected, transmitted;
          m_solver->solveIsoIso(reflected, transmitted);

          m_mainBeam->Dx[i]     = transmitted.Dx;
          m_mainBeam->Dy[i]     = transmitted.Dy;
          transmitted.vDx.copyToArray(m_mainBeam->vDx + 3 * i);

          m_splinterBeam->Dx[i] = reflected.Dx;
          m_splinterBeam->Dy[i] = reflected.Dy;
          reflected.vDx.copyToArray(m_splinterBeam->vDx + 3 * i);
        } else {
          m_mainBeam->prune(i);
          m_splinterBeam->prune(i);
        }
      } 
    }
  }
}

EMIsoIsoSolver::~EMIsoIsoSolver()
{

}

