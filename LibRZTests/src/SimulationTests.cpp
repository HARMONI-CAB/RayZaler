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
#define THIS_TEST_TAG "[Simulation]"

#include <catch2/catch_test_macros.hpp>
#include <Common.h>
#include <TopLevelModel.h>
#include <Simulation.h>
#include <RayTracingEngine.h>

using namespace RZ;

static const char *g_twoFlatMirrors = 
  "ApertureStop stop(diameter = .1);"

  "on aperture of stop {"
  "  translate(dy = .4) translate(dz = .5) rotate(45, 1, 0, 0) {"
  "    translate(dz = -.1)"
  "      FlatMirror M1(diameter = 1);"

  "    translate(dz = .1)"
  "      rotate(180, 1, 0, 0)"
  "      FlatMirror M2(diameter = 1);"
  
  "  }"
  "}";

TEST_CASE("Infinite reflection: side illumination", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_twoFlatMirrors);
  REQUIRE(model);

  auto stop = model->lookupOpticalElement("stop");
  REQUIRE(stop);

  auto frame = model->lookupReferenceFrame("stop.aperture");
  REQUIRE(frame != nullptr);

  std::list<Ray> rays;
  BeamProperties beamProp;
  Real focalLength = 0.2;
  Real objDistance = 2 * focalLength;
  Real diameter    = 0.05;

  Real idealFNum = objDistance / diameter;

  beamProp.id              = 0;
  beamProp.length          = 0;
  beamProp.diameter        = 5e-2;
  beamProp.offset          = Vec3::zero();
  beamProp.direction       = Vec3::eZ();
  beamProp.angularDiameter = 0;
  beamProp.numRays         = 1000;
  beamProp.shape           = Ring;
  beamProp.setPlaneRelative(frame);
  beamProp.collimate();
  beamProp.random          = false;
 

  OMModel::addBeam(rays, beamProp);
  REQUIRE(rays.size() == 1000);

  BeamTestStatistics inputStatistics;
  inputStatistics.computeFromRayList(rays, beamProp.direction);

  printf("(In) Infinite reflection: maximum radius: %g\n", inputStatistics.maxRad);
  printf("(In) Infinite reflection: center:         %g, %g\n", inputStatistics.x0, inputStatistics.y0);
  
  REQUIRE(releq(inputStatistics.maxRad, beamProp.diameter / 2));
  REQUIRE(isZero(inputStatistics.x0));
  REQUIRE(isZero(inputStatistics.y0));
  
  REQUIRE(model->traceNonSequential(rays, true));

  auto outRays = model->simulation()->engine()->getRays();

  REQUIRE(outRays.size() == beamProp.numRays);

  BeamTestStatistics outputStatistics;
  outputStatistics.computeFromRayList(outRays, beamProp.direction);

  printf("(Out) Infinite reflection: intercepted = %lld\n", outputStatistics.intercepted);
  printf("(Out) Infinite reflection: vignetted   = %lld\n", outputStatistics.vignetted);

  REQUIRE(outputStatistics.vignetted + outputStatistics.intercepted == beamProp.numRays);
  REQUIRE(outputStatistics.vignetted == beamProp.numRays);
  REQUIRE(outputStatistics.intercepted == 0);

  delete model;
}
