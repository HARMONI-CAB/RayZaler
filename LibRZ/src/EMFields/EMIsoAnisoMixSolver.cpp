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

#include <EMFields/EMIsoAnisoMixSolver.h>
#include <EMInterface.h>
#include <EMFields/EMSolver.h>
#include <Logger.h>

using namespace RZ;

EMIsoAnisoMixSolver::EMIsoAnisoMixSolver(
  const EMMedium *m1,
  const EMMedium *m2,
  const ReferenceFrame *parent) :
  EMInterfaceSolver(m1, m2, parent)
{
  
}

uint8_t
EMIsoAnisoMixSolver::secondaryBeamCount() const
{
  return 2; // RO and extraordinary
}

void
EMIsoAnisoMixSolver::transmit()
{
  const uint64_t oOff = 0;
  const uint64_t eOff = m_mainBeam->count;
  
  for (uint64_t i = m_currentSlice->start; i < m_currentSlice->end; ++i) {
    if (EMInterface::mustTransmitRay(m_mainBeam, i)) {
      Vec3 ui = Vec3(m_mainBeam->directions + 3 * i);
      Vec3 ki(m_mainBeam->k + i * 3);
      
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
        m_mainBeam->media[i] = m_solver->m2;
        m_solver->uo2.copyToArray(m_mainBeam->directions + 3 * i);
        m_solver->ko2.copyToArray(m_mainBeam->k          + 3 * i);
      } else {
        m_mainBeam->prune(i);
      }

      if (m_secondaryRays) {
        if (breakMask & ReflectedOrdinary) {
          m_splinterBeam->media[oOff + i] = m_solver->m1;
          m_solver->uo1.copyToArray(
            m_splinterBeam->directions + 3 * (oOff + i));
          m_solver->ko1.copyToArray(
            m_splinterBeam->k          + 3 * (oOff + i));
        } else {
          m_splinterBeam->prune(oOff + i);
        }

        if (breakMask & ReflectedExtraordinary) {
          m_splinterBeam->media[eOff + i] = m_solver->m1;
          m_solver->te1.copyToArray(
            m_splinterBeam->directions + 3 * (eOff + i));
          m_solver->ke1.copyToArray(
            m_splinterBeam->k          + 3 * (eOff + i));
        } else if (breakMask & TransmittedExtraordinary) {
          m_splinterBeam->media[eOff + i] = m_solver->m2;
          m_solver->te2.copyToArray(
            m_splinterBeam->directions + 3 * (eOff + i));
          m_solver->ke2.copyToArray(
            m_splinterBeam->k          + 3 * (eOff + i));
        } else {
          m_splinterBeam->prune(eOff + i);
        }
      }

      // Calculate fields, only for fully broken rays
      if (m_calculateFields) {
        EMFields to, ro, ex;
 
        if (breakMask == AllIsoAniso) {
          m_solver->solveIsoAniso(ro, to, ex);
        } else if (breakMask == AllAnisoIso) {
          m_solver->solveAnisoIso(ro, ex, to);
        } else {
          m_mainBeam->prune(i);
          m_splinterBeam->prune(oOff + i);
          m_splinterBeam->prune(eOff + i);

          continue;
        }

        m_mainBeam->Dx[i] = to.Dx;
        m_mainBeam->Dy[i] = to.Dy;
        to.vDx.copyToArray(m_mainBeam->vDx + 3 * i);

        m_splinterBeam->Dx[oOff + i] = ro.Dx;
        m_splinterBeam->Dy[oOff + i] = ro.Dy;
        ro.vDx.copyToArray(m_splinterBeam->vDx + 3 * (i + oOff));

        m_splinterBeam->Dx[eOff + i] = ex.Dx;
        m_splinterBeam->Dy[eOff + i] = ex.Dy;
        ex.vDx.copyToArray(m_splinterBeam->vDx + 3 * (i + eOff));

#if 0
        try {
          assertFields(i);
        } catch (std::runtime_error const &e) {
          RZError("NaN catched! Case: %s\n", breakMask == AllIsoAniso ? "Iso->Aniso" : "Aniso->Iso");
          RZError("  DReal:  %s\n", m_solver->DReal.toString().c_str());
          RZError("  DImag:  %s\n", m_solver->DImag.toString().c_str());
          RZError("  normal: %s\n", m_solver->normal.toString().c_str());
          RZError("  ws:     %s\n", m_solver->ws.toString().c_str());
          RZError("  is2:    %s\n", to.vDx.toString().c_str());
          RZError("  it2:    %s\n", to.vDy.toString().c_str());
          
          throw e;
        }
#endif
      }
    }
  }
}

EMIsoAnisoMixSolver::~EMIsoAnisoMixSolver()
{

}
