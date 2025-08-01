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

TEST_CASE("EMSolver: Aniso2Aniso", THIS_TEST_TAG)
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
  auto &dbg = solver.dirs;

  solver.setReferenceFrame(&frame);
  solver.setMedia(&m1, &m2);
  printf("Inverse dielectric tensor for medium 1:\n");
  printf("%s\n", solver.iep1.toString().c_str());
  putchar(10);

  printf("Inverse dielectric tensor for medium 2:\n");
  printf("%s\n", solver.iep2.toString().c_str());
  putchar(10);

  Vec3 fDx  = solver.iep1 * vDx;
  auto neff2 = 1 / (vDx * fDx);
  auto neff  = sqrt(neff2);

  REQUIRE(neff <= m1.ne);
  REQUIRE(m1.no <= neff);

  auto ki    = neff * ui;

  solver.setIncidentRay(ki, normal, Dx, 0, vDx);
  
  printf("Effective refractive index: %g\n", neff);
  printf("  <ki, ki>   = %g\n", ki * ki);
  printf("  ui         = %s\n", ui.toString().c_str());
  printf("  ki         = %s\n", ki.toString().c_str());
  printf("  vDx (iinc) = %s\n", vDx.toString().c_str());
  printf("  fDx        = %s\n", fDx.toString().c_str());
  REQUIRE(isZero(vDx * ui));
  putchar(10);

  printf("Checking proper ray break:\n");
  auto rays = solver.rayBreak();
  const char *descs[] = {"Evanescent", "Propagating"};
  printf("  Reflected   ordinary:      %s\n", descs[!!(rays & ReflectedOrdinary)]);
  printf("  Reflected   extraordinary: %s\n", descs[!!(rays & ReflectedExtraordinary)]);
  printf("  Transmitted ordinary:      %s\n", descs[!!(rays & TransmittedOrdinary)]);
  printf("  Transmitted extraordinary: %s\n", descs[!!(rays & TransmittedExtraordinary)]);
  REQUIRE(rays == AllRays);
  putchar(10);

  printf("Wave vectors:\n");
  printf("  ko1 = %s\n", solver.ko1.toString().c_str());
  printf("  ke1 = %s\n", solver.ke1.toString().c_str());
  printf("  ko2 = %s\n", solver.ko2.toString().c_str());
  printf("  ke2 = %s\n", solver.ke2.toString().c_str());

  putchar(10);

  printf("Refractive indices (guessed from k):\n");
  printf("  ni  = %g = %g\n", solver.ni, neff);
  REQUIRE(releq(solver.ni, neff));

  printf("  no1 = %g\n", solver.ko1.norm());
  printf("  ne1 = %g\n", solver.ke1.norm());
  printf("  no2 = %g\n", solver.ko2.norm());
  printf("  ne2 = %g\n", solver.ke2.norm());

  putchar(10);

  printf("Checking tranverse momentum conservation:\n");
  auto kiwq = ki * solver.wq;
  printf("  <ki,  wq> = %g\n", kiwq);
  auto ko1wq = solver.ko1 * solver.wq;
  printf("  <ko1, wq> = %g\n", ko1wq);
  REQUIRE(releq(kiwq, ko1wq));
  auto ke1wq = solver.ke1 * solver.wq;
  printf("  <ke1, wq> = %g\n", ke1wq);
  REQUIRE(releq(kiwq, ke1wq));
  auto ko2wq = solver.ko2 * solver.wq;
  printf("  <ko1, wq> = %g\n", ko2wq);
  REQUIRE(releq(kiwq, ko2wq));
  auto ke2wq = solver.ke2 * solver.wq;
  printf("  <ke2, wq> = %g\n", ke2wq);
  REQUIRE(releq(kiwq, ke2wq));
  putchar(10);

  printf("Solving fields...\n");
  solver.solveAnisoAniso(ro, re, to, te);

  Real D1, D2;
  Real B1, B2;

  Vec3 E1, E2;
  Vec3 H1, H2;
  
  Vec3 Si, So1, Se1, So2, Se2;
  Real S1, S2;

  printf("Check field directions:\n");
  printf("  io1 = %s\n", dbg.io1.toString().c_str());
  REQUIRE(isZero(dbg.io1 * solver.uo1));

  printf("  ie1 = %s\n", dbg.ie1.toString().c_str());
  REQUIRE(isZero(dbg.ie1 * solver.ue1));

  printf("  io2 = %s\n", dbg.io2.toString().c_str());
  REQUIRE(isZero(dbg.io2 * solver.uo2));

  printf("  ie2 = %s\n", dbg.ie2.toString().c_str());
  REQUIRE(isZero(dbg.ie2 * solver.ue2));
  putchar(10);

  printf("Checking normal D field continuity (in-phase)\n");
  Real Do1 = dbg.DR.coords[0];
  Real De1 = dbg.DR.coords[1];
  Real Do2 = dbg.DR.coords[2];
  Real De2 = dbg.DR.coords[3];

  D1 = (dbg.DiR + Do1 * dbg.io1 + De1 * dbg.ie1) * normal;
  D2 = (Do2 * dbg.io2 + De2 * dbg.ie2) * normal;
  printf("  DiR      = %s\n", dbg.DiR.toString().c_str());
  printf("  DR       = %s\n", dbg.DR.toString().c_str());
  printf("  <D1, n>  = %g\n", D1);
  printf("  <D2, n>  = %g\n", D2);

  REQUIRE(releq(D1, D2));
  putchar(10);

  printf("Checking tangent E field continuity (in-phase)\n");
  Vec3 Ei  = solver.DampR * dbg.fiR;
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

  printf("Checking tangent H field continuity (in-phase)\n");
  Vec3 Hi  = solver.DampR * dbg.giR;
  Vec3 Ho1 = Do1 * dbg.go1;
  Vec3 He1 = De1 * dbg.ge1;
  Vec3 Ho2 = Do2 * dbg.go2;
  Vec3 He2 = De2 * dbg.ge2;

  H1 = (Hi + Ho1 + He1).cross(normal);
  H2 = (Ho2 + He2).cross(normal);
  printf("  Hi  = %s\n", Hi.toString().c_str());
  printf("  Ho1 = %s\n", Ho1.toString().c_str());
  printf("  He1 = %s\n", He1.toString().c_str());
  printf("  Ho2 = %s\n", Ho2.toString().c_str());
  printf("  He2 = %s\n", He2.toString().c_str());

  printf("  H1 x n = %s\n", H1.toString().c_str());
  printf("  H2 x n = %s\n", H2.toString().c_str());
  REQUIRE(isZero((H1 - H2).norm()));
  putchar(10);

  printf("Checking normal B field continuity (in-phase)\n");
  B1 = (Hi + Ho1 + He1) * normal;
  B2 = (Ho2 + He2) * normal;
  printf("  <B1, n>  = %g\n", B1);
  printf("  <B2, n>  = %g\n", B2);
  REQUIRE(releq(B1, B2));
  putchar(10);

  printf("Checking energy conservation (in-phase)\n");
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

  printf("Checking normal D field continuity (quadrature)\n");
  Do1 = dbg.DI.coords[0];
  De1 = dbg.DI.coords[1];
  Do2 = dbg.DI.coords[2];
  De2 = dbg.DI.coords[3];

  D1 = (dbg.DiI + Do1 * dbg.io1 + De1 * dbg.ie1) * normal;
  D2 = (Do2 * dbg.io2 + De2 * dbg.ie2) * normal;
  printf("  DiI      = %s\n", dbg.DI.toString().c_str());
  printf("  DI       = %s\n", dbg.DI.toString().c_str());
  printf("  <D1, n>  = %g\n", D1);
  printf("  <D2, n>  = %g\n", D2);

  REQUIRE(releq(D1, D2));
  putchar(10);
}



