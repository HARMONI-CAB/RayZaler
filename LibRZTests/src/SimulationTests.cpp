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
#include <Elements/RayBeamElement.h>

using namespace RZ;

static const char *g_twoFlatMirrors = 
  "ApertureStop stop(diameter = .1);"

  "on aperture of stop {"
  "  translate(dy = .375) translate(dz = -.5) rotate(-45, 1, 0, 0) {"
  "    translate(dz = -.1)"
  "      FlatMirror M1(diameter = 1);"

  "    translate(dz = .1)"
  "      rotate(180, 1, 0, 0)"
  "      FlatMirror M2(diameter = 1);"
  
  "  }"
  "}";

static const char *g_rotatedFocusLens = 
  "dof K(-4, 4) = -1;"
  "dof focalLength(.1, .3) = .2;"
  "dof D = 5e-2;"
  "dof angle(0, 180) = 0;"
  "dof thickness = 2e-3;"
  
  "var fp = .5 * thickness + focalLength;"
  "var op = .5 * thickness + 2 * focalLength;"
  
  "rotate(angle, 1, 0, 0) ConicLens L1("
    "thickness   = thickness,"
    "conic       = K,"
    "focalLength = focalLength,"
    "diameter    = D);"
  
  "translate(dz = -fp) Detector bfpDet(flip = true);"
  "translate(dz = -op) Detector imgDet(flip = true);"
  "translate(dz = op) port object;"
  
  "path bfp L1 to bfpDet;"
  "path img L1 to imgDet;";


TEST_CASE("Infinite reflection: stray light", THIS_TEST_TAG)
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
  beamProp.direction       = -Vec3::eZ();
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
  REQUIRE(model->beam()->strayRays() == beamProp.numRays);

  delete model;
}

TEST_CASE("Infinite reflection: limited propagation", THIS_TEST_TAG)
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
  beamProp.direction       = -Vec3::eZ();
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
  
  REQUIRE(model->traceNonSequential(rays, true, nullptr, false, nullptr, true, 2));

  auto outRays = model->simulation()->engine()->getRays();

  REQUIRE(outRays.size() == beamProp.numRays);

  BeamTestStatistics outputStatistics;
  outputStatistics.computeFromRayList(outRays, beamProp.direction);

  printf("(Out) Infinite reflection: intercepted = %lld\n", outputStatistics.intercepted);
  printf("(Out) Infinite reflection: vignetted   = %lld\n", outputStatistics.vignetted);

  REQUIRE(outputStatistics.vignetted + outputStatistics.intercepted == beamProp.numRays);
  REQUIRE(outputStatistics.vignetted   == 0);
  REQUIRE(outputStatistics.intercepted == beamProp.numRays);

  delete model;
}

TEST_CASE("Rotated lens: proper orientation", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_rotatedFocusLens);
  REQUIRE(model);

  auto L1 = model->lookupOpticalElement("L1");
  REQUIRE(L1);

  auto surfaces = L1->opticalSurfaces();
  REQUIRE(surfaces.size() == 2);

  auto detector = model->lookupOpticalElement("imgDet");
  REQUIRE(detector);

  surfaces = detector->opticalSurfaces();
  REQUIRE(surfaces.size() == 1);

  auto fp = surfaces.front();
  REQUIRE(fp != nullptr);

  auto object = model->lookupReferenceFrame("object");
  REQUIRE(object != nullptr);

  // Record hits!
  detector->setRecordHits(true);

  std::list<Ray> rays;
  BeamProperties beamProp;
  Real focalLength = 0.2;
  Real objDistance = 2 * focalLength;
  Real diameter    = 0.05;

  Real idealFNum = objDistance / diameter;

  beamProp.id              = 0;
  beamProp.length          = 1;
  beamProp.diameter        = 0;
  beamProp.offset          = Vec3::zero();
  beamProp.direction       = -Vec3::eZ();
  beamProp.angularDiameter = 0;
  beamProp.numRays         = 1000;
  beamProp.shape           = Point;
  beamProp.setPlaneRelative(object);
  beamProp.collimate();
  beamProp.setObjectFNum(idealFNum);
  beamProp.objectShape     = RingLike;
  beamProp.random          = false;
 
  printf("(obj) Rotated lens: aperture angle: %g deg\n", rad2deg(beamProp.angularDiameter));

  OMModel::addBeam(rays, beamProp);
  REQUIRE(rays.size() == 1000);

  BeamTestStatistics inputStatistics;
  inputStatistics.computeFromRayList(rays, beamProp.direction);
  printf("(obj) Rotated lens: object radius: %g\n", inputStatistics.maxRad);
  printf("(obj) Rotated lens: object center: %g, %g\n", inputStatistics.x0, inputStatistics.y0);
  
  printf(
    "(obj) Rotated lens: object f/#: %g (ideal = %g, err = %g)\n",
    inputStatistics.fNum,
    idealFNum,
    inputStatistics.fNum - idealFNum);
  REQUIRE(releq(inputStatistics.fNum, idealFNum));
  REQUIRE(isZero(inputStatistics.x0));
  REQUIRE(isZero(inputStatistics.y0));
  
  REQUIRE(model->setDof("D", diameter + 1e-3)); // Extra clearance to avoid edge rays
  REQUIRE(model->setDof("focalLength", focalLength));

  REQUIRE(model->traceNonSequential(rays));

  // Do statistics on the image
  BeamTestStatistics statistics;
  statistics.computeFromSurface(fp);
  printf("(obj) Rotated lens: image radius: %g\n", statistics.maxRad);
  printf("(obj) Rotated lens: image center: %g, %g\n", statistics.x0, statistics.y0);
  printf("(obj) Rotated lens: image f/#: %g (ideal %g)\n", statistics.fNum, idealFNum);
  
  // Require f/# within 2% error
  REQUIRE(fp->hits.size() == rays.size());
  REQUIRE(isZero(statistics.x0));
  REQUIRE(isZero(statistics.y0));
  REQUIRE(statistics.maxRad < 3e-4); // Room for aberrations
  REQUIRE(releq(statistics.fNum, idealFNum, 2e-2));
  
  delete model;
}

TEST_CASE("Rotated lens: 180 deg flip", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_rotatedFocusLens);
  REQUIRE(model);

  auto L1 = model->lookupOpticalElement("L1");
  REQUIRE(L1);

  auto surfaces = L1->opticalSurfaces();
  REQUIRE(surfaces.size() == 2);

  auto detector = model->lookupOpticalElement("imgDet");
  REQUIRE(detector);

  surfaces = detector->opticalSurfaces();
  REQUIRE(surfaces.size() == 1);

  auto fp = surfaces.front();
  REQUIRE(fp != nullptr);

  auto object = model->lookupReferenceFrame("object");
  REQUIRE(object != nullptr);

  // Record hits!
  detector->setRecordHits(true);

  // Flip lens
  REQUIRE(model->setDof("angle", 180.));

  std::list<Ray> rays;
  BeamProperties beamProp;
  Real focalLength = 0.2;
  Real objDistance = 2 * focalLength;
  Real diameter    = 0.05;

  Real idealFNum = objDistance / diameter;

  beamProp.id              = 0;
  beamProp.length          = 1;
  beamProp.diameter        = 0;
  beamProp.offset          = Vec3::zero();
  beamProp.direction       = -Vec3::eZ();
  beamProp.angularDiameter = 0;
  beamProp.numRays         = 1000;
  beamProp.shape           = Point;
  beamProp.setPlaneRelative(object);
  beamProp.collimate();
  beamProp.setObjectFNum(idealFNum);
  beamProp.objectShape     = RingLike;
  beamProp.random          = false;
 
  printf("(obj) Rotated lens: aperture angle: %g deg\n", rad2deg(beamProp.angularDiameter));

  OMModel::addBeam(rays, beamProp);
  REQUIRE(rays.size() == 1000);

  BeamTestStatistics inputStatistics;
  inputStatistics.computeFromRayList(rays, beamProp.direction);
  printf("(obj) Rotated lens: object radius: %g\n", inputStatistics.maxRad);
  printf("(obj) Rotated lens: object center: %g, %g\n", inputStatistics.x0, inputStatistics.y0);
  
  printf(
    "(obj) Rotated lens: object f/#: %g (ideal = %g, err = %g)\n",
    inputStatistics.fNum,
    idealFNum,
    inputStatistics.fNum - idealFNum);
  REQUIRE(releq(inputStatistics.fNum, idealFNum));
  REQUIRE(isZero(inputStatistics.x0));
  REQUIRE(isZero(inputStatistics.y0));
  
  REQUIRE(model->setDof("D", diameter + 1e-3)); // Extra clearance to avoid edge rays
  REQUIRE(model->setDof("focalLength", focalLength));

  REQUIRE(model->traceNonSequential(rays));

  // Do statistics on the image
  BeamTestStatistics statistics;
  statistics.computeFromSurface(fp);
  printf("(obj) Rotated lens: image radius: %g\n", statistics.maxRad);
  printf("(obj) Rotated lens: image center: %g, %g\n", statistics.x0, statistics.y0);
  printf("(obj) Rotated lens: image f/#: %g (ideal %g)\n", statistics.fNum, idealFNum);
  
  // Require f/# within 2% error
  REQUIRE(fp->hits.size() == rays.size());
  REQUIRE(isZero(statistics.x0));
  REQUIRE(isZero(statistics.y0));
  REQUIRE(statistics.maxRad < 3e-4); // Room for aberrations
  REQUIRE(releq(statistics.fNum, idealFNum, 2e-2));
  
  delete model;
}

