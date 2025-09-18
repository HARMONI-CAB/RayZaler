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
#include <EMFields/EMAnisoAnisoSolver.h>
#include <EMFields/EMIsoAnisoMixSolver.h>

using namespace RZ;

EMInterfaceSolver::EMInterfaceSolver(
  const EMMedium *m1,
  const EMMedium *m2,
  const ReferenceFrame *frame)
{
  m_solver = new EMSolver();

  m_solver->setReferenceFrame(frame);
  
  setMedia(m1, m2);
}

EMInterfaceSolver *
EMInterfaceSolver::make(
  const EMMedium *m1,
  const EMMedium *m2,
  const ReferenceFrame *frame)
{
  if (m1->isotropic() && m2->isotropic())
    return new EMIsoIsoSolver(m1, m2, frame);
  else if (!m1->isotropic() && !m2->isotropic())
    return new EMAnisoAnisoSolver(m1, m2, frame);
  else
    return new EMIsoAnisoMixSolver(m1, m2, frame);
}

void
EMInterfaceSolver::setMedia(const EMMedium *m1, const EMMedium *m2)
{
  m_m1 = m1;
  m_m2 = m2;
  m_solver->setMedia(m1, m2);
}

static inline void
assertBeamFields(const RayBeam *beam, uint64_t i, uint64_t off, std::string const &name)
{
    if (std::isnan(beam->Dx[i + off].real()))
    throw std::runtime_error(string_printf("NaN detected: Dx[%d].real() [%s]", i, name.c_str()));

  if (std::isnan(beam->Dx[i + off].imag()))
    throw std::runtime_error(string_printf("NaN detected: Dx[%d].imag() [%s]", i, name.c_str()));

  if (std::isnan(beam->Dy[i + off].real()))
    throw std::runtime_error(string_printf("NaN detected: Dy[%d].real() [%s]", i, name.c_str()));

  if (std::isnan(beam->Dy[i + off].imag()))
    throw std::runtime_error(string_printf("NaN detected: Dy[%d].imag() [%s]", i, name.c_str()));
}

void
EMInterfaceSolver::assertFields(uint64_t i) const
{
  assertBeamFields(m_mainBeam, i, 0, "main beam");

  for (auto j = 0; j < m_sCount; ++j)
    assertBeamFields(
      m_splinterBeam,
      i,
      m_mainBeam->count * j,
      string_printf("splinter #%d", j + 1));
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

    auto splinterRays = m_mainBeam->count * m_sCount;
    if (splinterBeam->count < splinterRays)
      splinterBeam->allocate(splinterRays);

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

