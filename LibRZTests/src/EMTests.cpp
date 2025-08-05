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

#define CATCH_CONFIG_MAIN
#define THIS_TEST_TAG "[EM]"

#include <catch2/catch_test_macros.hpp>
#include <Common.h>
#include <TopLevelModel.h>
#include <Simulation.h>
#include <RayTracingEngine.h>
#include <Elements/RayBeamElement.h>
#include <EMFields/EMSolver.h>

using namespace RZ;


static const char *g_fresnelWindow =
  "parameter n2 = 2;"
  "parameter distance = 1;"
  "parameter pxsz = 1e-3;"
  
  "dof incAdjust(-20, 20) = 0;"
  "parameter thickness = .5;"
  
  "var theta_B = incAdjust + rad2deg(atan(n2));"
  "var theta_2 = rad2deg(asin(sin(deg2rad(theta_B)) / n2));"
  
  "CircularWindow window(diameter = 2.15, thickness = thickness, n = n2);"
  
  "translate(dz = thickness / 2 + 1e-3) Detector input(pixelWidth = 5e-3, pixelHeight = 5e-3, flip = true);"
  "translate(dz = thickness / 2 - 1e-3) Detector output(pixelWidth = 5e-3, pixelHeight = 5e-3, flip = true);"
  
  "translate(dz = thickness / 2) {"
  "  rotate(theta_B, 1, 0, 0) translate(dz = distance) {"
  "    port input;"
  "    Detector incidenceDet(pixelWidth = pxsz, pixelHeight = pxsz, flip = true);"
  "  }"
  "  rotate(-theta_B, 1, 0, 0) translate(dz = distance) Detector reflectionDet(pixelWidth = pxsz, pixelHeight = pxsz);"
  "}"
  
  "translate(dz = thickness/2) rotate(theta_2+ 180, 1, 0, 0) translate(dz = thickness/2) {"
  "  Detector transmissionDet(pixelWidth = pxsz, pixelHeight = pxsz);"
  "}";

TEST_CASE("Fresnel equations: power conservation (Iso2Iso)", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_fresnelWindow);
  REQUIRE(model);

  auto input = model->lookupOpticalElement("input");
  REQUIRE(input);
  input->setRecordHits(true);

  auto output = model->lookupOpticalElement("output");
  REQUIRE(output);
  output->setRecordHits(true);

  auto frame = model->lookupReferenceFrame("incidenceDet.surface");
  REQUIRE(frame != nullptr);

  auto inputSurf = input->lookupSurface("detSurf");
  REQUIRE(inputSurf != nullptr);

  auto outputSurf = output->lookupSurface("detSurf");
  REQUIRE(outputSurf != nullptr);

  Real angles[] = {-15, 0, +15.};

  for (int i = 0; i < 3; ++i) {
    RayList rays;
    BeamProperties beamProp;
    TracingProperties tracing;

    model->setDof("incAdjust", angles[i]);
    beamProp.id              = 0;
    beamProp.length          = 1;
    beamProp.diameter        = 4e-2;
    beamProp.offset          = Vec3::zero();
    beamProp.direction       = -Vec3::eZ();
    beamProp.angularDiameter = 0;
    beamProp.numRays         = 20;
    beamProp.shape           = Circular;
    
    beamProp.setPlaneRelative(frame);
    beamProp.collimate();
    beamProp.random          = true;
  
    OMModel::addBeam(rays, beamProp);
    REQUIRE(rays.size() == beamProp.numRays);

    tracing.type            = NonSequential;
    tracing.pRays           = &rays;
    tracing.maxPropagations = 20;
    tracing.keepStrayRays   = true;
    tracing.calculateFields = true;
    tracing.secondaryRays   = true;

    BeamTestStatistics inStat;
    BeamTestStatistics outStat;

    input->clearHits();
    output->clearHits();
    REQUIRE(model->simulation()->trace(tracing));

    inStat.computeFromSurface(inputSurf);
    outStat.computeFromSurface(outputSurf);

    printf(
      "(%+3g deg) Power conservation (iso2iso): %4.1f dB in -> %4.1f dB out\n",
      angles[i],
      10 * log10(inStat.incidentPower),
      10 * log10(outStat.incidentPower));
    
    REQUIRE(inStat.incidentPower != 0.);
    REQUIRE(inStat.incidentPower > 0);
    REQUIRE(outStat.incidentPower != 0.);
    REQUIRE(outStat.incidentPower > 0.);
    REQUIRE(releq(inStat.incidentPower, outStat.incidentPower));
  }

  delete model;
}

TEST_CASE("Fresnel equations: Brewster's angle (Iso2Iso)", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_fresnelWindow);
  REQUIRE(model);

  auto input = model->lookupOpticalElement("incidenceDet");
  REQUIRE(input);
  input->setRecordHits(true);

  auto output = model->lookupOpticalElement("reflectionDet");
  REQUIRE(output);
  output->setRecordHits(true);

  auto frame = model->lookupReferenceFrame("incidenceDet.surface");
  REQUIRE(frame != nullptr);

  auto inputSurf = input->lookupSurface("detSurf");
  REQUIRE(inputSurf != nullptr);

  auto outputSurf = output->lookupSurface("detSurf");
  REQUIRE(outputSurf != nullptr);

  Real angles[] = {-15, 0, +15.};

  for (int i = 0; i < 3; ++i) {
    RayList rays;
    BeamProperties beamProp;
    TracingProperties tracing;

    model->setDof("incAdjust", angles[i]);
    beamProp.id              = 0;
    beamProp.length          = 1;
    beamProp.diameter        = 4e-2;
    beamProp.offset          = Vec3::zero();
    beamProp.direction       = -Vec3::eZ();
    beamProp.angularDiameter = 0;
    beamProp.numRays         = 100;
    beamProp.shape           = Circular;
    
    beamProp.setPlaneRelative(frame);
    beamProp.collimate();
    beamProp.random          = true;
  
    OMModel::addBeam(rays, beamProp);
    REQUIRE(rays.size() == beamProp.numRays);

    tracing.type            = NonSequential;
    tracing.pRays           = &rays;
    tracing.maxPropagations = 20;
    tracing.keepStrayRays   = true;
    tracing.calculateFields = true;
    tracing.secondaryRays   = true;

    BeamTestStatistics inStat;
    BeamTestStatistics outStat;

    input->clearHits();
    output->clearHits();
    REQUIRE(model->simulation()->trace(tracing));

    inStat.computeFromSurface(inputSurf);
    outStat.computeFromSurface(outputSurf);

    REQUIRE(inStat.S[0] > 0);
    inStat.S[1] /= inStat.S[0];
    inStat.S[2] /= inStat.S[0];
    inStat.S[3] /= inStat.S[0];

    REQUIRE(outStat.S[0] > 0);
    outStat.S[0] /= inStat.S[0];
    outStat.S[1] /= inStat.S[0];
    outStat.S[2] /= inStat.S[0];
    outStat.S[3] /= inStat.S[0];

    printf(
      "(%+3g deg) S(in) = [%5.2f, %+5.2f, %+5.2f, %+5.2f] -> S(out) = [%5.2f, %+5.2f, %+5.2f, %+5.2f]\n",
      angles[i],
      1.,  inStat.S[1],  inStat.S[2],  inStat.S[3],
      outStat.S[0], outStat.S[1], outStat.S[2], outStat.S[3]);

    if (isZero(angles[i])) {
      REQUIRE(releq(outStat.S[0], outStat.S[1]));
      REQUIRE(isZero(outStat.S[2]));
      REQUIRE(isZero(outStat.S[3]));
    } else {
      REQUIRE(outStat.S[0] > outStat.S[1]);
      REQUIRE(!isZero(outStat.S[2]));
      REQUIRE(!isZero(outStat.S[3]));
    }
  }

  delete model;
}

static inline void
verifyIncidentRay(EMSolver const &solver)
{
  printf("Incident ray properties:\n");
  printf("  <ki, ki>   = %g (ni = %g)\n", solver.ki * solver.ki, solver.ni);
  printf("  ui         = %s\n", solver.ui.toString().c_str());
  printf("  ki         = %s\n", solver.ki.toString().c_str());
  printf("  vDx (iinc) = %s\n", solver.viDx.toString().c_str());
  printf("  ws         = %s\n", solver.ws.toString().c_str());
  printf("  wq         = %s\n", solver.wq.toString().c_str());
  
  REQUIRE(isZero(solver.viDx * solver.ui));
  REQUIRE(isZero(solver.wq * solver.normal));
  REQUIRE(isZero(solver.ws * solver.normal));
  REQUIRE(isZero(solver.ws * solver.wq));

  putchar(10);
}

static inline void
verifyRayBreak(EMSolver const &solver, int desiredMask)
{
  auto rays  = solver.rayMask;
  
  const char *descs[] = {"Evanescent", "Propagating"};

  printf("Ray break result:\n");

  if (solver.m1->isotropic()) {
    if (desiredMask & ReflectedOrdinary)
      printf("  Reflected:     %s\n", descs[!!(rays & ReflectedOrdinary)]);
  } else {
    if (desiredMask & ReflectedOrdinary)
      printf("  Reflected (O): %s\n", descs[!!(rays & ReflectedOrdinary)]);

    if (desiredMask & ReflectedExtraordinary)
      printf("  Reflected (X): %s\n", descs[!!(rays & ReflectedExtraordinary)]);
  }

  if (solver.m2->isotropic()) {
    if (desiredMask & TransmittedOrdinary)
      printf("  Transmitted:   %s\n", descs[!!(rays & TransmittedOrdinary)]);
  } else {
    if (desiredMask & TransmittedOrdinary)
      printf("  Transmitted (O): %s\n", descs[!!(rays & TransmittedOrdinary)]);

    if (desiredMask & TransmittedExtraordinary)
      printf("  Transmitted (X): %s\n", descs[!!(rays & TransmittedExtraordinary)]);
  }

  REQUIRE(rays == desiredMask);
  putchar(10);

  printf("Wave vectors:\n");

  if (solver.m1->isotropic()) {
    if (desiredMask & ReflectedOrdinary) {
      printf("  kr  = %s\n", solver.ko1.toString().c_str());
      REQUIRE(solver.ko1 * solver.normal > 0);
    }
  } else {
    if (desiredMask & ReflectedOrdinary) {
      printf("  ko1 = %s\n", solver.ko1.toString().c_str());
      REQUIRE(solver.ko1 * solver.normal > 0);
    }

    if (desiredMask & ReflectedExtraordinary) {
      printf("  ke1 = %s\n", solver.ke1.toString().c_str());
      REQUIRE(solver.ke1 * solver.normal > 0);
    }
  }

  if (solver.m2->isotropic()) {
    if (desiredMask & TransmittedOrdinary) {
      printf("  kt  = %s\n", solver.ko2.toString().c_str());
      REQUIRE(solver.ko2 * solver.normal < 0);
    }
  } else {
    if (desiredMask & TransmittedOrdinary) {
      printf("  ko2 = %s\n", solver.ko2.toString().c_str());
      REQUIRE(solver.ko2 * solver.normal < 0);
    }

    if (desiredMask & TransmittedExtraordinary) {
      printf("  ke2 = %s\n", solver.ke2.toString().c_str());
      REQUIRE(solver.ke2 * solver.normal < 0);
    }
  }

  putchar(10);

  printf("Refractive indices (guessed from k):\n");
  printf("  ni  = %g\n", solver.ni);

  if (solver.m1->isotropic()) {
    if (desiredMask & ReflectedOrdinary)
      printf("  n1  = %g\n", solver.ko1.norm());
  } else {
    if (desiredMask & ReflectedOrdinary)
      printf("  no1 = %g\n", solver.ko1.norm());

    if (desiredMask & ReflectedExtraordinary)
      printf("  ne1 = %g\n", solver.ke1.norm());
  }

  if (solver.m2->isotropic()) {
    if (desiredMask & ReflectedOrdinary)
      printf("  n2  = %g\n", solver.ko2.norm());
  } else {
    if (desiredMask & TransmittedOrdinary)
      printf("  no2 = %g\n", solver.ko2.norm());

    if (desiredMask & TransmittedExtraordinary)
      printf("  ne2 = %g\n", solver.ke2.norm());
  }

  putchar(10);

  printf("Checking tranverse momentum conservation:\n");
  auto kiwq = solver.ki * solver.wq;
  printf("  <ki,  wq> = %g\n", kiwq);

  if (solver.m1->isotropic()) {
    if (desiredMask & ReflectedOrdinary) {
      auto ko1wq = solver.ko1 * solver.wq;
      printf("  <kr,  wq> = %g\n", ko1wq);
      REQUIRE(releq(kiwq, ko1wq));
    }
  } else {
    if (desiredMask & ReflectedOrdinary) {
      auto ko1wq = solver.ko1 * solver.wq;
      printf("  <ko1, wq> = %g\n", ko1wq);
      REQUIRE(releq(kiwq, ko1wq));
    }

    if (desiredMask & ReflectedExtraordinary) {
      auto ke1wq = solver.ke1 * solver.wq;
      printf("  <ke1, wq> = %g\n", ke1wq);
      REQUIRE(releq(kiwq, ke1wq));
    }
  }

  if (solver.m2->isotropic()) {
    if (desiredMask & TransmittedOrdinary) {
      auto ko2wq = solver.ko2 * solver.wq;
      printf("  <kt,  wq> = %g\n", ko2wq);
      REQUIRE(releq(kiwq, ko2wq));
    }
  } else {
    if (desiredMask & TransmittedOrdinary) {
      auto ko2wq = solver.ko2 * solver.wq;
      printf("  <ko1, wq> = %g\n", ko2wq);
      REQUIRE(releq(kiwq, ko2wq));
    }

    if (desiredMask & TransmittedExtraordinary) {
      auto ke2wq = solver.ke2 * solver.wq;
      printf("  <ke2, wq> = %g\n", ke2wq);
      REQUIRE(releq(kiwq, ke2wq));
    }
  }

  putchar(10);
}

static void
verifyFieldsDirections(EMSolver const &solver)
{
  auto &dbg = solver.dirs;

  printf("D-field directions:\n");

  if (solver.m1->isotropic()) {
    if (solver.rayMask & ReflectedOrdinary) {
      printf("  is1 = %s\n", dbg.is1.toString().c_str());
      REQUIRE(isZero(dbg.is1 * solver.uo1));
      printf("  it1 = %s\n", dbg.it1.toString().c_str());
      REQUIRE(isZero(dbg.it1 * solver.uo1));
      REQUIRE(isZero(dbg.is1 * dbg.it1));
    }
  } else {
    if (solver.rayMask & ReflectedOrdinary) {
      printf("  io1 = %s\n", dbg.io1.toString().c_str());
      REQUIRE(isZero(dbg.io1 * solver.uo1));
    }

    if (solver.rayMask & ReflectedExtraordinary) {
      printf("  ie1 = %s\n", dbg.ie1.toString().c_str());
      REQUIRE(isZero(dbg.ie1 * solver.ue1));
    }
  }

  if (solver.m2->isotropic()) {
    if (solver.rayMask & TransmittedOrdinary) {
      printf("  is2 = %s\n", dbg.is2.toString().c_str());
      REQUIRE(isZero(dbg.is2 * solver.uo2));
      printf("  it2 = %s\n", dbg.it2.toString().c_str());
      REQUIRE(isZero(dbg.it2 * solver.uo2));
      REQUIRE(isZero(dbg.is2 * dbg.it2));
    }
  } else {
    if (solver.rayMask & TransmittedOrdinary) {
      printf("  io2 = %s\n", dbg.io2.toString().c_str());
      REQUIRE(isZero(dbg.io2 * solver.uo2));
    }

    if (solver.rayMask & TransmittedExtraordinary) {
      printf("  ie2 = %s\n", dbg.ie2.toString().c_str());
      REQUIRE(isZero(dbg.ie2 * solver.ue2));
    }
  }
  putchar(10);

  printf("E-field directions:\n");
  if (solver.m1->isotropic()) {
    if (solver.rayMask & ReflectedOrdinary) {
      printf("  fs1 = %s\n", dbg.fs1.toString().c_str());
      printf("  ft1 = %s\n", dbg.it1.toString().c_str());
    }
  } else {
    if (solver.rayMask & ReflectedOrdinary) {
      printf("  fo1 = %s\n", dbg.fo1.toString().c_str());
    }

    if (solver.rayMask & ReflectedExtraordinary) {
      printf("  fe1 = %s\n", dbg.fe1.toString().c_str());
    }
  }

  if (solver.m2->isotropic()) {
    if (solver.rayMask & TransmittedOrdinary) {
      printf("  fs2 = %s\n", dbg.fs2.toString().c_str());
      printf("  ft2 = %s\n", dbg.ft2.toString().c_str());
    }
  } else {
    if (solver.rayMask & TransmittedOrdinary) {
      printf("  fo2 = %s\n", dbg.fo2.toString().c_str());
    }

    if (solver.rayMask & TransmittedExtraordinary) {
      printf("  fe2 = %s\n", dbg.fe2.toString().c_str());
    }
  }

  putchar(10);

  printf("H-field directions:\n");
  if (solver.m1->isotropic()) {
    if (solver.rayMask & ReflectedOrdinary) {
      printf("  gs1 = %s\n", dbg.gs1.toString().c_str());
      REQUIRE(isZero(dbg.gs1 * solver.uo1));
      REQUIRE(isZero(dbg.gs1 * dbg.fs1));
      
      printf("  gt1 = %s\n", dbg.gt1.toString().c_str());
      REQUIRE(isZero(dbg.gt1 * solver.uo1));
      REQUIRE(isZero(dbg.gt1 * dbg.ft1));
    }
  } else {
    if (solver.rayMask & ReflectedOrdinary) {
      printf("  go1 = %s\n", dbg.go1.toString().c_str());
      REQUIRE(isZero(dbg.go1 * solver.uo1));
      REQUIRE(isZero(dbg.go1 * dbg.fo1));
    }

    if (solver.rayMask & ReflectedExtraordinary) {
      printf("  ge1 = %s\n", dbg.ge1.toString().c_str());
      REQUIRE(isZero(dbg.ge1 * solver.ue1));
      REQUIRE(isZero(dbg.ge1 * dbg.fe1));
    }
  }

  if (solver.m2->isotropic()) {
    if (solver.rayMask & TransmittedOrdinary) {
      printf("  gs2 = %s\n", dbg.gs2.toString().c_str());
      REQUIRE(isZero(dbg.gs2 * solver.uo2));
      REQUIRE(isZero(dbg.gs2 * dbg.fs2));
      printf("  gt2 = %s\n", dbg.gt2.toString().c_str());
      REQUIRE(isZero(dbg.gt2 * solver.uo2));
      REQUIRE(isZero(dbg.gt2 * dbg.ft2));
    }
  } else {
    if (solver.rayMask & TransmittedOrdinary) {
      printf("  go2 = %s\n", dbg.go2.toString().c_str());
      REQUIRE(isZero(dbg.go2 * solver.uo2));
      REQUIRE(isZero(dbg.go2 * dbg.fo2));
    }

    if (solver.rayMask & TransmittedExtraordinary) {
      printf("  ge2 = %s\n", dbg.ge2.toString().c_str());
      REQUIRE(isZero(dbg.ge2 * solver.ue2));
      REQUIRE(isZero(dbg.ge2 * dbg.fe2));
    }
  }

  putchar(10);
}

static void
verifyFields(
  EMSolver const &solver,
  bool imag)
{
  auto &dbg = solver.dirs;
  auto const &normal = solver.normal;
  const char *component = imag ? "quadrature" : "in-phase";

  Real D1, D2;
  Real B1, B2;

  Vec3 E1, E2;
  Vec3 H1, H2;
  
  Vec3 Si, So1, Se1, So2, Se2;
  Real S1, S2;

  const Vec3 &Di = imag ? dbg.DiI      : dbg.DiR;
  const Vec4 &D  = imag ? dbg.DI       : dbg.DR;
  auto Damp      = imag ? solver.DampI : solver.DampR;
  const Vec3 &fi = imag ? dbg.fiI      : dbg.fiR;
  const Vec3 &gi = imag ? dbg.giI      : dbg.giR;

  printf("Checking normal D field continuity (%s)\n", component);
  Real Do1 = D.coords[0];
  Real De1 = D.coords[1];
  Real Do2 = D.coords[2];
  Real De2 = D.coords[3];

  D1 = (Di + Do1 * dbg.io1 + De1 * dbg.ie1) * normal;
  D2 = (Do2 * dbg.io2 + De2 * dbg.ie2) * normal;
  printf("  DiR      = %s\n", Di.toString().c_str());
  printf("  DR       = %s\n", D.toString().c_str());
  printf("  <D1, n>  = %g\n", D1);
  printf("  <D2, n>  = %g\n", D2);

  REQUIRE(releq(D1, D2));
  putchar(10);

  printf("Checking tangent E field continuity (%s)\n", component);
  Vec3 Ei  = Damp * fi;
  Vec3 Eo1 = Do1 * dbg.fo1;
  Vec3 Ee1 = De1 * dbg.fe1;
  Vec3 Eo2 = Do2 * dbg.fo2;
  Vec3 Ee2 = De2 * dbg.fe2;

  E1 = (Ei + Eo1 + Ee1).cross(normal);
  E2 = (Eo2 + Ee2).cross(normal);
  
  printf("  E1 x n = %s\n", E1.toString().c_str());
  printf("  E2 x n = %s\n", E2.toString().c_str());
  REQUIRE(isZero((E1 - E2).norm()));
  putchar(10);

  printf("Checking tangent H field continuity (%s)\n", component);
  Vec3 Hi  = Damp * gi;
  Vec3 Ho1 = Do1 * dbg.go1;
  Vec3 He1 = De1 * dbg.ge1;
  Vec3 Ho2 = Do2 * dbg.go2;
  Vec3 He2 = De2 * dbg.ge2;

  H1 = (Hi + Ho1 + He1).cross(normal);
  H2 = (Ho2 + He2).cross(normal);

  printf("  H1 x n = %s\n", H1.toString().c_str());
  printf("  H2 x n = %s\n", H2.toString().c_str());
  REQUIRE(isZero((H1 - H2).norm()));
  putchar(10);

  printf("Checking normal B field continuity (%s)\n", component);
  B1 = (Hi + Ho1 + He1) * normal;
  B2 = (Ho2 + He2) * normal;
  printf("  <B1, n>  = %g\n", B1);
  printf("  <B2, n>  = %g\n", B2);
  REQUIRE(releq(B1, B2));
  putchar(10);

  printf("Checking energy conservation (%s)\n", component);
  Si  = Ei.cross(Hi);
  So1 = Eo1.cross(Ho1);
  Se1 = Ee1.cross(He1);
  So2 = Eo2.cross(Ho2);
  Se2 = Ee2.cross(He2);

  S1 = (Si + So1 + Se1) * normal;
  S2 = (So2 + Se2) * normal;
  printf("  <S1, n>  = %g\n", S1);
  printf("  <S2, n>  = %g\n", S2);
  REQUIRE(releq(S1, S2));
  putchar(10);

  printf("Checking Poynting vector alignment (%s)\n", component);
  REQUIRE(Si  * normal < 0);
  REQUIRE(So2 * normal < 0);
  REQUIRE(So2 * normal < 0);

  REQUIRE(So1 * normal > 0);
  REQUIRE(Se1 * normal > 0);
  
  if (solver.rayMask & BirefringentRays) {
    if (solver.rayMask & ReflectedExtraordinary) {
      auto norm  = solver.te1.norm();
      auto power = Se1.norm();

      printf("  Se1 ray direction: %s\n", Se1.normalized().toString().c_str());
      printf("  Se1 power:         %g\n", power);
      printf("  te1 vector:        %s\n", solver.te1.toString().c_str());

      REQUIRE(releq(norm, 1));
      REQUIRE(releq(solver.te1 * Se1, power));
    }
    
    if (solver.rayMask & TransmittedExtraordinary) {
      auto norm  = solver.te2.norm();
      auto power = Se2.norm();

      printf("  Se2 ray direction: %s\n", Se2.normalized().toString().c_str());
      printf("  Se2 power:         %g\n", power);
      printf("  te2 vector:        %s\n", solver.te2.toString().c_str());

      REQUIRE(releq(norm, 1));
      REQUIRE(releq(solver.te2 * Se2, power));
    }
  }

  putchar(10);
}

TEST_CASE("EMSolver: Iso2Iso", THIS_TEST_TAG)
{  
  // Reference frame: just a world frame
  WorldFrame frame("world");

  // Medium 1
  EMMedium m1;

  m1.type  = EMMediumIsotropic;
  m1.n     = 1.1;
  REQUIRE(m1.isotropic());

  printf("Medium 1 n:    %g\n", m1.n);

  // Medium 2
  EMMedium m2;

  m2.type  = EMMediumIsotropic;
  m2.n     = 2.2;
  REQUIRE(m2.isotropic());

  printf("Medium 2 n:    %g\n", m2.n);

  Vec3 normal = -Vec3::eY();
  Real angle  = deg2rad(35);
  Vec3 ui     = Vec3(sin(angle), cos(angle), 0);

  // Ordinary ray simulation
  Vec3 vDx = ui.cross(Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN)).normalized();
  Complex Dx(10, 20);
  Complex Dy(30, 40);

  while (vDx * vDx < 1e-12)
    vDx = ui.cross(Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN)).normalized();

  REQUIRE(isZero(vDx * ui));
  
  EMSolver solver;
  EMFields reflected, transmitted;
  
  solver.debug = true;

  solver.setReferenceFrame(&frame);
  solver.setMedia(&m1, &m2);

  auto neff = m1.n;
  auto ki   = neff * ui;

  solver.setIncidentRay(ki, normal, Dx, Dy, vDx);
  
  verifyIncidentRay(solver);

  solver.rayBreak();

  verifyRayBreak(solver, ReflectedOrdinary | TransmittedOrdinary);

  solver.solveIsoIso(reflected, transmitted);
  
  verifyFieldsDirections(solver);
  verifyFields(solver, false);
  verifyFields(solver, true);
}

TEST_CASE("EMSolver: Aniso2Aniso (Ordinary ray)", THIS_TEST_TAG)
{  
  // Reference frame: just a world frame
  WorldFrame frame("world");

  // Medium 1
  EMMedium m1;

  m1.type  = EMMediumUniaxial;
  m1.frame = &frame;
  m1.axis  = Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN).normalized();
  m1.no    = 1.1;
  m1.ne    = 1.5;

  REQUIRE(!m1.isotropic());

  printf("Medium 1 axis: %s\n", m1.axis.toString().c_str());
  printf("Medium 1 no:   %g\n", m1.no);
  printf("Medium 1 ne:   %g\n", m1.ne);

  // Medium 2
  EMMedium m2;

  m2.type  = EMMediumUniaxial;
  m2.frame = &frame;
  m2.axis  = Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN).normalized();
  m2.no    = 2.2;
  m2.ne    = 2.5;
  REQUIRE(!m2.isotropic());

  printf("Medium 2 axis: %s\n", m2.axis.toString().c_str());
  printf("Medium 2 no:   %g\n", m2.no);
  printf("Medium 2 ne:   %g\n", m2.ne);

  Vec3 normal = -Vec3::eY();
  Real angle  = deg2rad(35);
  Vec3 ui     = Vec3(sin(angle), cos(angle), 0);

  // Ordinary ray simulation
  Vec3 vDx = ui.cross(m1.axis).normalized();
  Complex Dx(10, 20);

  while (vDx * vDx < 1e-12) {
    m1.axis  = Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN).normalized();
    vDx = ui.cross(m1.axis).normalized();
  }

  EMSolver solver;
  EMFields ro, re, to, te;
  
  solver.debug = true;

  solver.setReferenceFrame(&frame);
  solver.setMedia(&m1, &m2);

  printf("NOTE: INCIDENT RAY IS ORDINARY RAY\n");

  auto neff = m1.no;
  auto ki   = neff * ui;

  solver.setIncidentRay(ki, normal, Dx, 0, vDx);
  
  verifyIncidentRay(solver);

  solver.rayBreak();

  verifyRayBreak(solver, AllRays);

  solver.solveAnisoAniso(ro, re, to, te);
  
  verifyFieldsDirections(solver);
  verifyFields(solver, false);
  verifyFields(solver, true);
}


TEST_CASE("EMSolver: Aniso2Aniso (Extraordinary ray)", THIS_TEST_TAG)
{  
  // Reference frame: just a world frame
  WorldFrame frame("world");

  // Medium 1
  EMMedium m1;

  m1.type  = EMMediumUniaxial;
  m1.frame = &frame;
  m1.axis  = Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN).normalized();
  m1.no    = 1.1;
  m1.ne    = 1.5;

  REQUIRE(!m1.isotropic());

  printf("Medium 1 axis: %s\n", m1.axis.toString().c_str());
  printf("Medium 1 no:   %g\n", m1.no);
  printf("Medium 1 ne:   %g\n", m1.ne);

  // Medium 2
  EMMedium m2;

  m2.type  = EMMediumUniaxial;
  m2.frame = &frame;
  m2.axis  = Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN).normalized();
  m2.no    = 2.2;
  m2.ne    = 2.5;
  REQUIRE(!m2.isotropic());

  printf("Medium 2 axis: %s\n", m2.axis.toString().c_str());
  printf("Medium 2 no:   %g\n", m2.no);
  printf("Medium 2 ne:   %g\n", m2.ne);

  Vec3 normal = -Vec3::eY();
  Real angle  = deg2rad(35);
  Vec3 ui     = Vec3(sin(angle), cos(angle), 0);

  // Extraordinary ray simulation
  Vec3 vDx = ui.cross(m1.axis).cross(ui).normalized();
  Complex Dx(10, 20);

  while (vDx * vDx < 1e-12) {
    m1.axis  = Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN).normalized();
    vDx = ui.cross(m1.axis).cross(ui).normalized();
  }

  EMSolver solver;
  EMFields ro, re, to, te;
  
  solver.debug = true;

  solver.setReferenceFrame(&frame);
  solver.setMedia(&m1, &m2);

  Vec3 fDx  = solver.iep1 * vDx;
  auto neff2 = 1 / (vDx * fDx);
  auto neff  = sqrt(neff2);

  REQUIRE(neff <= m1.ne);
  REQUIRE(m1.no <= neff);

  printf("NOTE: INCIDENT RAY IS EXTRA-ORDINARY RAY. SNELL WILL NOT APPLY.\n");
  auto ki    = neff * ui;

  solver.setIncidentRay(ki, normal, Dx, 0, vDx);
  
  verifyIncidentRay(solver);

  solver.rayBreak();

  verifyRayBreak(solver, AllRays);

  solver.solveAnisoAniso(ro, re, to, te);
  
  verifyFieldsDirections(solver);
  verifyFields(solver, false);
  verifyFields(solver, true);
}

TEST_CASE("EMSolver: Aniso2Iso (Ordinary ray)", THIS_TEST_TAG)
{  
  // Reference frame: just a world frame
  WorldFrame frame("world");

  // Medium 1
  EMMedium m1;

  m1.type  = EMMediumUniaxial;
  m1.frame = &frame;
  m1.axis  = Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN).normalized();
  m1.no    = 1.1;
  m1.ne    = 1.5;

  REQUIRE(!m1.isotropic());

  printf("Medium 1 axis: %s\n", m1.axis.toString().c_str());
  printf("Medium 1 no:   %g\n", m1.no);
  printf("Medium 1 ne:   %g\n", m1.ne);

  // Medium 2
  EMMedium m2;

  m2.type  = EMMediumIsotropic;
  m2.n     = 2.2;

  REQUIRE(m2.isotropic());

  printf("Medium 2 n:    %g\n", m2.n);

  Vec3 normal = -Vec3::eY();
  Real angle  = deg2rad(35);
  Vec3 ui     = Vec3(sin(angle), cos(angle), 0);

  // Ordinary ray simulation
  Vec3 vDx = ui.cross(m1.axis).normalized();
  Complex Dx(10, 20);

  while (vDx * vDx < 1e-12) {
    m1.axis  = Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN).normalized();
    vDx = ui.cross(m1.axis).normalized();
  }

  EMSolver solver;
  EMFields ro, re, transmitted;
  
  solver.debug = true;

  solver.setReferenceFrame(&frame);
  solver.setMedia(&m1, &m2);

  printf("NOTE: INCIDENT RAY IS ORDINARY RAY\n");

  auto neff = m1.no;
  auto ki   = neff * ui;

  solver.setIncidentRay(ki, normal, Dx, 0, vDx);
  
  verifyIncidentRay(solver);

  solver.rayBreak();

  verifyRayBreak(
    solver,
    ReflectedOrdinary | ReflectedExtraordinary | TransmittedOrdinary);

  solver.solveAnisoIso(ro, re, transmitted);
  
  verifyFieldsDirections(solver);
  verifyFields(solver, false);
  verifyFields(solver, true);
}

TEST_CASE("EMSolver: Aniso2Iso (Extraordinary ray)", THIS_TEST_TAG)
{  
  // Reference frame: just a world frame
  WorldFrame frame("world");

  // Medium 1
  EMMedium m1;

  m1.type  = EMMediumUniaxial;
  m1.frame = &frame;
  m1.axis  = Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN).normalized();
  m1.no    = 1.1;
  m1.ne    = 1.5;

  REQUIRE(!m1.isotropic());

  printf("Medium 1 axis: %s\n", m1.axis.toString().c_str());
  printf("Medium 1 no:   %g\n", m1.no);
  printf("Medium 1 ne:   %g\n", m1.ne);

  // Medium 2
  EMMedium m2;

  m2.type  = EMMediumIsotropic;
  m2.n     = 2.2;

  REQUIRE(m2.isotropic());

  printf("Medium 2 n:    %g\n", m2.n);

  Vec3 normal = -Vec3::eY();
  Real angle  = deg2rad(35);
  Vec3 ui     = Vec3(sin(angle), cos(angle), 0);

  // Extraordinary ray simulation
  Vec3 vDx = ui.cross(m1.axis).cross(ui).normalized();
  Complex Dx(10, 20);

  while (vDx * vDx < 1e-12) {
    m1.axis  = Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN).normalized();
    vDx = ui.cross(m1.axis).cross(ui).normalized();
  }

  EMSolver solver;
  EMFields ro, re, transmitted;
  
  solver.debug = true;

  solver.setReferenceFrame(&frame);
  solver.setMedia(&m1, &m2);

  Vec3 fDx  = solver.iep1 * vDx;
  auto neff2 = 1 / (vDx * fDx);
  auto neff  = sqrt(neff2);

  REQUIRE(neff <= m1.ne);
  REQUIRE(m1.no <= neff);

  printf("NOTE: INCIDENT RAY IS EXTRA-ORDINARY RAY. SNELL WILL NOT APPLY.\n");
  auto ki    = neff * ui;

  solver.setIncidentRay(ki, normal, Dx, 0, vDx);
  
  verifyIncidentRay(solver);

  solver.rayBreak();

  verifyRayBreak(
    solver,
    ReflectedOrdinary | ReflectedExtraordinary | TransmittedOrdinary);

  solver.solveAnisoIso(ro, re, transmitted);
  
  verifyFieldsDirections(solver);
  verifyFields(solver, false);
  verifyFields(solver, true);
}

TEST_CASE("EMSolver: Iso2Aniso", THIS_TEST_TAG)
{  
  // Reference frame: just a world frame
  WorldFrame frame("world");

  // Medium 1
  EMMedium m1;

  m1.type  = EMMediumIsotropic;
  m1.n     = 1.1;

  REQUIRE(m1.isotropic());

  printf("Medium 1 n:    %g\n", m1.n);

  // Medium 2
  EMMedium m2;

  m2.type  = EMMediumUniaxial;
  m2.frame = &frame;
  m2.axis  = Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN).normalized();
  m2.no    = 2.2;
  m2.ne    = 2.5;
  REQUIRE(!m2.isotropic());

  printf("Medium 2 axis: %s\n", m2.axis.toString().c_str());
  printf("Medium 2 no:   %g\n", m2.no);
  printf("Medium 2 ne:   %g\n", m2.ne);

  Vec3 normal = -Vec3::eY();
  Real angle  = deg2rad(35);
  Vec3 ui     = Vec3(sin(angle), cos(angle), 0);

  // Ordinary ray simulation
  Vec3 vDx = ui.cross(Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN)).normalized();
  Complex Dx(10, 20);
  Complex Dy(30, 40);

  while (vDx * vDx < 1e-12)
    vDx = ui.cross(Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN)).normalized();

  REQUIRE(isZero(vDx * ui));

  EMSolver solver;
  EMFields reflected, to, te;
  
  solver.debug = true;

  solver.setReferenceFrame(&frame);
  solver.setMedia(&m1, &m2);

  auto neff = m1.n;
  auto ki   = neff * ui;

  solver.setIncidentRay(ki, normal, Dx, Dy, vDx);
  
  verifyIncidentRay(solver);

  solver.rayBreak();

  verifyRayBreak(
    solver,
    ReflectedOrdinary | TransmittedOrdinary | TransmittedExtraordinary);

  solver.solveIsoAniso(reflected, to, te);
  
  verifyFieldsDirections(solver);
  verifyFields(solver, false);
  verifyFields(solver, true);
}
