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

#include <EMFields/EMIsoIsoSolver.h>
#include <EMInterface.h>
#include <EMFields/EMSolver.h>

using namespace RZ;

EMIsoIsoSolver::EMIsoIsoSolver(
  const EMMedium *m1,
  const EMMedium *m2,
  const ReferenceFrame *parent) :
  EMInterfaceSolver(m1, m2, parent)
{

}

uint8_t
EMIsoIsoSolver::secondaryBeamCount() const
{
  return 1; // The only reflected ray
}

void
EMIsoIsoSolver::transmit()
{
  Vec3 dummy;

  for (uint64_t i = m_currentSlice->start; i < m_currentSlice->end; ++i) {
    if (EMInterface::mustTransmitRay(m_mainBeam, i)) {
      Vec3 ui = Vec3(m_mainBeam->directions + 3 * i);
      Vec3 ki(m_mainBeam->k + i * 3);
      
      // Configure incident ray
      m_solver->setIncidentRay(
        ki,
        Vec3(m_mainBeam->normals + 3 * i),
        m_calculateFields ? m_mainBeam->Dx[i]  : 0,
        m_calculateFields ? m_mainBeam->Dy[i]  : 0,
        m_calculateFields ? Vec3(m_mainBeam->vDx + 3 * i) : dummy);
      
      // Calculate transmitted and reflected rays
      auto breakMask = m_solver->rayBreak();

      if (breakMask & TransmittedOrdinary) {
        m_mainBeam->media[i] = m_solver->m2;
        m_solver->uo2.copyToArray(m_mainBeam->directions + 3 * i);
        m_solver->ko2.copyToArray(m_mainBeam->k          + 3 * i);
      } else {
        m_mainBeam->prune(i);
      }

      if (m_secondaryRays) {
        if (breakMask & ReflectedOrdinary) {
          m_splinterBeam->media[i] = m_solver->m1;
          m_solver->uo1.copyToArray(m_splinterBeam->directions + 3 * i);
          m_solver->ko1.copyToArray(m_splinterBeam->k          + 3 * i);
        } else {
          m_splinterBeam->prune(i);
          
        }
      } else if (!m_mainBeam->hasRay(i)) {
        m_mainBeam->unprune(i);
        m_mainBeam->media[i] = m_solver->m1;
        m_solver->uo1.copyToArray(m_mainBeam->directions + 3 * i);
        m_solver->ko1.copyToArray(m_mainBeam->k          + 3 * i);
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

