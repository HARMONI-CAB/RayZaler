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

inline bool
DielectricEMInterface::G(
  Real &bfr,
  Real &rad,
  Vec3 const &k_i,
  Vec3 const &normal,
  Real n_o,
  Real n_e,
  Vec3 const &axis)
{
  auto qj = (n_e * n_e - n_o * n_o) / (n_o * n_o);
  auto etaaj = -normal * axis;
  auto kieta = -k_i * normal;
  auto kiaj  = k_i * axis;

  auto a     = 1 + qj * etaaj * etaaj;
  auto inv2a = .5 / a;
  auto b     = 2 * (kieta + qj * kiaj * etaaj);
  auto c     = k_i * k_i - n_e * n_e + qj * kiaj * kiaj;

  auto D     = b * b - 4 * a * c;

  if (D < 0)
    return false;

  bfr = -b * inv2a;
  rad = sqrt(D) * inv2a;

  return true;
}

inline bool
DielectricEMInterface::anisoAnisoBreak(
        Vec3 &sor,
        Vec3 &ser,
        Vec3 &sot,
        Vec3 &set,
        Vec3 const &k_i,
        Vec3 const &normal,
        Real n_o1,
        Real n_e1,
        Vec3 const &axis1,
        Real n_o2,
        Real n_e2,
        Vec3 const &axis2)
{
  Real bfr1, rad1, bfr2, rad2;

  if (!G(bfr1, rad1, k_i, normal, n_o1, n_e1, axis1))
    return false;

  if (!G(bfr2, rad2, k_i, normal, n_o2, n_e2, axis1))
    return false;

  auto G1 = bfr1 - rad1;
  auto G2 = bfr2 + rad2;

  auto k_r = k_i - G1 * normal;
  auto k_t = k_i - G2 * normal;

  auto iKR = 1 / k_r.norm();
  auto iKT = 1 / k_t.norm();

  sor = n_o1 * k_r;
  sot = n_o2 * k_t;

  auto kra1 = k_r * axis1;
  auto kta2 = k_t * axis2;

  ser = n_e1 * n_e1 * kra1 * iKR * axis1 + n_o1 * n_o1 * (iKR * (k_r - kra1 * axis1));
  set = n_e2 * n_e2 * kta2 * iKT * axis2 * n_o2 * n_o2 * (iKT * (k_t - kta2 * axis2));

  return true;
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
  const Vec3 viEx(inputBeam->vDx          + 3 * inputRay);
  
  bool positive = ui * normal >= 0;
  const Real n1 = positive ? m_n2 : m_n1;
  const Real n2 = positive ? m_n1 : m_n2;

  const Real invn1sq  = 1 / (n1 * n1);
  const Real n2ton1sq = n2 * n2 * invn1sq;

  const Complex Dx = inputBeam->Dx[inputRay];
  const Complex Dy = inputBeam->Dy[inputRay];


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
  auto Dir  = Dx.real() * viEx + Dy.real() * viEy;
  auto Dii  = Dx.imag() * viEx + Dy.imag() * viEy;

  auto Dis  = Complex(Dir * ws,  Dii * ws);
  auto Dip  = Complex(Dir * wip, Dii * wip);

  // Calculation of the field amplitudes of the transmitted ray, in the SxP plane
  auto Dts = ts * Dis;
  auto Dtp = tp * Dip;

  // Update transmitted ray
  ws.copyToArray(inputBeam->vDx    + 3 * inputRay);
  inputBeam->Dx[inputRay] = Dts * n2ton1sq;
  inputBeam->Dy[inputRay] = Dtp * n2ton1sq;

  if (splinterBeam != nullptr) {
    const Vec3 ur(splinterBeam->directions + 3 * splinterRay);
    auto wrp = ur.cross(ws).normalized();

    // Calculation of the field amplitudes of the reflected ray, in the SxP plane
    auto Drs = rs * Dis;
    auto Drp = rp * Dip;

    ws.copyToArray(splinterBeam->vDx + 3 * splinterRay);
    splinterBeam->Dx[splinterRay] = Drs;
    splinterBeam->Dy[splinterRay] = Drp;
  }
}

//
// ISOTROPIC TO ISOTROPIC CASE
// 
// This is regular Snell + Fresnel. We identify two media: the positive
// medium and the negative medium. The positive medium is the side the normal
// points at. The positive medium refractive index is n1, and the negative medium
// refractive index is n2.

inline void
DielectricEMInterface::transmitIsoIso(
  RayBeamSlice const &slice,
  RayBeam *splinterBeam)
{
  auto beam = slice.beam;
  Real rdir = m_n1n2, rinv = 1 / m_n1n2;
  Real n1   = m_n1;
  Real n2   = m_n2;

  // Allocate space for secondary rays
  if (splinterBeam != nullptr) {
    if (splinterBeam->count < beam->count)
      splinterBeam->allocate(beam->count);

    slice.copyTo(RayBeamSlice(splinterBeam, slice.start, slice.end));
  }

  for (auto i = slice.start; i < slice.end; ++i) if (mustTransmitRay(beam, i)) {
    const Vec3 ui(beam->directions  + 3 * i);
    const Vec3 normal(beam->normals + 3 * i);
    auto medium = beam->media[i];
    auto iSign  = ui * normal;
    
    // Sanity check.
    assert(medium == (iSign < 0 ? pMedium() : nMedium()));

    const Vec3 transmitted = iSign < 0 
      ? snell(ui, normal, rdir)
      : snell(ui, -normal, rinv);

    auto tSign = transmitted * normal;

    transmitted.copyToArray(beam->directions + 3 * i);
    
    beam->media[i] = tSign < 0 ? nMedium() : pMedium();
    beam->neff[i]  = beam->media[i]->n;

    if (splinterBeam != nullptr) {
      // Calculate secondary ray if the primary ray is a transmission. 
      // Otherwise (total reflection) prune secondary ray
      if (iSign * tSign > 0)
        reflection(ui, normal).copyToArray(splinterBeam->directions + 3 * i);
      else
        splinterBeam->prune(i);
    }

    if (beam->fields)
      calcIsoToIsoFields(beam, i, splinterBeam, i, ui);
  }
}

void
DielectricEMInterface::transmit(
  RayBeamSlice const &slice,
  RayBeam *splinterBeam)
{
  blockLight(slice); // Prune rays according to transmission

  switch (m_interfaceCase) {
    case IsoToIso:
      transmitIsoIso(slice, splinterBeam);
      break;

    default:
      // TODO: Write me
      break;
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
DielectricEMInterface::setRefractiveIndex(Real in, Real out)
{
  m_n1   = in;
  m_n2   = out;
  m_n1n2 = in / out;
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
