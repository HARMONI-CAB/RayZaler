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

using namespace RZ;

std::string
DielectricEMInterface::name() const
{
  return "DielectricEMInterface";
}

void
DielectricEMInterface::setRefractiveIndex(Real in, Real out)
{
  m_n1    = in;
  m_n2   = out;
  m_n1n2 = in / out;
}

inline void
DielectricEMInterface::calcIsoToIsoFields(
  RayBeam *inputBeam,
  uint64_t inputRay,
  RayBeam *splinterBeam,
  uint64_t splinterRay,
  const Vec3 &ui)
{
  const Vec3 normal(inputBeam->normals    + 3 * inputRay);
  const Vec3 ut(inputBeam->directions     + 3 * inputRay);
  const Vec3 viEx(inputBeam->uEx          + 3 * inputRay);
  const Complex Ex = inputBeam->Ex[inputRay];
  const Complex Ey = inputBeam->Ey[inputRay];

  bool positive = ui * normal >= 0;
  const Real n1 = positive ? m_n1 : m_n2;
  const Real n2 = positive ? m_n2 : m_n1;

  auto ws = ui.cross(normal);
  
  if (ws.isNull()) {
    auto axis1 = normal.cross(Vec3::eX());
    auto axis2 = normal.cross(Vec3::eY());

    if (axis1 * axis1 > axis2 * axis2)
      ws = axis1;
    else
      ws = axis2;
  }

  // Calculation of different basis vectors. The SxP plane is derived from the
  // (right-handed) orthogonal triad:
  //
  //  X -> ws                 (S)
  //  Y -> wip, wrp or wtp    (P)
  //  Z -> ui,  ur  or ut     (Direction)
  //

  ws        = ws.normalized();
  auto wq   = normal.cross(ws).normalized();
  auto uin  = ui * normal;
  auto utn  = ut * normal;
  auto viEy = ui.cross(viEx);

  /////////////////// Reflection and transmission coefficients /////////////////
  // Secant component
  auto rs  = (n1 * uin - n2 * utn) / (n1 * uin + n2 * utn);
  auto ts  = (2 * n1 * uin)        / (n1 * uin + n2 * utn);

  // Parallel component
  auto rp  = (n2 * uin - n1 * utn) / (n2 * uin + n1 * utn);
  auto tp  = (2 * n1 * uin)        / (n2 * uin + n1 * utn);

  // Deduction of the parallel components of each ray
  auto wip = ui.cross(ws).normalized();
  auto wtp = ut.cross(ws).normalized();

  // Projection of the incident electric field amplitudes onto the SxP plane
  auto Eir  = Ex.real() * viEx + Ey.real() * viEy;
  auto Eii  = Ex.imag() * viEx + Ey.imag() * viEy;

  auto Eis  = Complex(Eir * ws,  Eii * ws);
  auto Eip  = Complex(Eir * wip, Eii * wip);

  // Calculation of the field amplitudes of the transmitted ray, in the SxP plane
  auto Ets = ts * Eis;
  auto Etp = tp * Eip;

  // Update transmitted ray
  ws.copyToArray(inputBeam->uEx    + 3 * inputRay);
  inputBeam->Ex[inputRay] = Ets;
  inputBeam->Ey[inputRay] = Etp;

  if (splinterBeam != nullptr) {
    const Vec3 ur(splinterBeam->directions + 3 * splinterRay);
    auto wrp = ur.cross(ws).normalized();

    // Calculation of the field amplitudes of the reflected ray, in the SxP plane
    auto Ers = rs * Eis;
    auto Erp = rp * Eip;

    ws.copyToArray(splinterBeam->uEx + 3 * splinterRay);
    splinterBeam->Ex[splinterRay] = Ers;
    splinterBeam->Ey[splinterRay] = Erp;
  }
}


void
DielectricEMInterface::transmit(
  RayBeamSlice const &slice,
  RayBeam *splinterBeam)
{
  blockLight(slice); // Prune rays according to transmission

  auto inputBeam = slice.beam;
  Real rdir = m_n1n2, rinv = 1 / m_n1n2;
  Real n1  = m_n1;
  Real n2  = m_n2;

  if (splinterBeam != nullptr) {
    // Make sure the splinter beam is properly allocated
    if (splinterBeam->count < inputBeam->count) {
      switch (m_interfaceCase) {
        case IsoToIso:
          splinterBeam->allocate(inputBeam->count);
          break;

        default:
          // TODO: Write me!
          break;
      }
    }

    // Copy splinter beam
    switch (m_interfaceCase) {
      case IsoToIso:
        slice.copyTo(RayBeamSlice(splinterBeam, slice.start, slice.end));
        break;

      default:
        // TODO: Write me
        break;
    }
  }

  for (auto i = slice.start; i < slice.end; ++i) {
    if (mustTransmitRay(slice.beam, i)) {
      const Vec3 ui(inputBeam->directions  + 3 * i);
      const Vec3 normal(inputBeam->normals + 3 * i);
      auto medium = inputBeam->media[i];

      if (ui * normal < 0) {
        snell(ui, normal, rdir).copyToArray(inputBeam->directions + 3 * i);
        inputBeam->media[i] = nMedium();
      } else {
        snell(ui, -normal, rinv).copyToArray(inputBeam->directions + 3 * i);
        inputBeam->media[i] = pMedium();
      }

      if (splinterBeam != nullptr)
        reflection(ui, normal).copyToArray(splinterBeam->directions + 3 * i);

      if (inputBeam->fields) switch (m_interfaceCase) {
        case IsoToIso:
          calcIsoToIsoFields(inputBeam, i, splinterBeam, i, ui);
          break;

        default:
          // TODO: Write me!
          break;
      }
    }
  }
}

DielectricEMInterface::~DielectricEMInterface()
{

}

bool
DielectricEMInterface::detectInterfaceCase()
{
  bool pIso = pMedium() == nullptr || pMedium()->isotropic();
  bool nIso = nMedium() == nullptr || nMedium()->isotropic();

  if (pIso && nIso)
    m_interfaceCase = IsoToIso;
  else if (pIso && !nIso)
    m_interfaceCase = IsoToAniso;
  else if (!pIso && nIso)
    m_interfaceCase = AnisoToIso;
  else
    m_interfaceCase = AnisoToAniso;
  
  return m_interfaceCase == IsoToIso;
}

void
DielectricEMInterface::setSurroundingMedium(const EMMedium *medium)
{
  EMInterface::setSurroundingMedium(medium);

  if (!detectInterfaceCase()) {
    RZWarning(
      "Anisotropic media are not compatible with DielectricEMInterface.\n");
    RZWarning(
      "Electric field amplitudes will not be calculated.\n");
  }

  setRefractiveIndex(pMedium()->n, nMedium()->n);
}

void
DielectricEMInterface::setMedia(
  const EMMedium *positive,
  const EMMedium *negative)
{
  EMInterface::setMedia(positive, negative);

  if (!detectInterfaceCase()) {
    RZWarning(
      "Anisotropic media are not compatible with DielectricEMInterface\n");
    RZWarning(
      "Electric field amplitudes will not be calculated.\n");
  }

  setRefractiveIndex(pMedium()->n, nMedium()->n);
}
