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

#include <EMFields/EMAnisoAnisoSolver.h>
#include <EMInterface.h>
#include <EMFields/EMSolver.h>

using namespace RZ;

EMAnisoAnisoSolver::EMAnisoAnisoSolver(
  const EMMedium *m1,
  const EMMedium *m2,
  const ReferenceFrame *parent) :
  EMInterfaceSolver(m1, m2, parent)
{
  
}

uint8_t
EMAnisoAnisoSolver::secondaryBeamCount() const
{
  return 3; // RO, RE, TE
}

void
EMAnisoAnisoSolver::transmit()
{
  const uint64_t roOff = 0;
  const uint64_t reOff = m_splinterBeam->count;
  const uint64_t teOff = 2 * m_splinterBeam->count;
  
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

      if (m_secondaryRays) {
        if (breakMask & ReflectedOrdinary) {
          m_splinterBeam->neff[roOff + i]  = m_solver->m1->no;
          m_splinterBeam->media[roOff + i] = m_solver->m1;
          m_solver->uo1.copyToArray(
            m_splinterBeam->directions + 3 * (roOff + i));
        } else {
          m_splinterBeam->prune(roOff + i);
        }

        if (breakMask & ReflectedExtraordinary) {
          m_splinterBeam->neff[reOff + i]  = m_solver->nee1;
          m_splinterBeam->media[reOff + i] = m_solver->m1;
          m_solver->uo1.copyToArray(
            m_splinterBeam->directions + 3 * (reOff + i));
        } else {
          m_splinterBeam->prune(reOff + i);
        }

        if (breakMask & TransmittedExtraordinary) {
          m_splinterBeam->neff[teOff + i]  = m_solver->nee2;
          m_splinterBeam->media[teOff + i] = m_solver->m2;
          m_solver->uo1.copyToArray(
            m_splinterBeam->directions + 3 * (teOff + i));
        } else {
          m_splinterBeam->prune(teOff + i);
        }
      }

      // Calculate fields, only for fully broken rays
      if (m_calculateFields) {
        if (breakMask == AllRays) {
          EMFields to, te, ro, re;
          m_solver->solveAnisoAniso(ro, re, to, te);

          m_mainBeam->Dx[i]     = to.Dx;
          m_mainBeam->Dy[i]     = to.Dy;
          to.vDx.copyToArray(m_mainBeam->vDx + 3 * i);

          m_splinterBeam->Dx[roOff + i] = ro.Dx;
          m_splinterBeam->Dy[roOff + i] = ro.Dy;
          ro.vDx.copyToArray(m_splinterBeam->vDx + 3 * (i + roOff));

          m_splinterBeam->Dx[reOff + i] = re.Dx;
          m_splinterBeam->Dy[reOff + i] = re.Dy;
          re.vDx.copyToArray(m_splinterBeam->vDx + 3 * (i + reOff));

          m_splinterBeam->Dx[teOff + i] = te.Dx;
          m_splinterBeam->Dy[teOff + i] = te.Dy;
          te.vDx.copyToArray(m_splinterBeam->vDx + 3 * (i + teOff));
        } else {
          m_mainBeam->prune(i);
          m_splinterBeam->prune(roOff + i);
          m_splinterBeam->prune(reOff + i);
          m_splinterBeam->prune(teOff + i);
          
        }
      } 
    }
  }
}

EMAnisoAnisoSolver::~EMAnisoAnisoSolver()
{

}

