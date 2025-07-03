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
