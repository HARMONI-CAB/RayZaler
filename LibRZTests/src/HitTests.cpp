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

#define CATCH_CONFIG_MAIN
#define THIS_TEST_TAG "[HitDiagrams]"

#include <catch2/catch_test_macros.hpp>
#include <TopLevelModel.h>

using namespace RZ;

static const char *g_parabolicReflectorCode = 
  "dof focalLength = 1;"
  "dof D           = 1;"

  "ParabolicMirror M1("
  "  focalLength = focalLength,"
  "  diameter    = D,"
  "  thickness   = 1e-2);"

  "on vertex of M1 translate(dz = focalLength) Detector det;"
  "path M1 to det;";

static const char *g_focusLens = 
  "dof K(-4, 4) = -1;"
  "dof focalLength(.1, .3) = .2;"
  "dof D = 5e-2;"

  "ConicLens L1("
  "  thickness   = 2e-3,"
  "  conic       = K,"
  "  focalLength = focalLength,"
  "  diameter    = D);"

  "on backFocalPlane of L1 Detector bfpDet;"
  "on imagePlane of L1 Detector imgDet;"
  "on objectPlane of L1 port object;"

  "path bfp L1 to bfpDet;"
  "path img L1 to imgDet;";

static const char *g_idealFocusLens = 
  "dof focalLength(.1, .3) = .2;"
  "dof D = 5e-2;"

  "IdealLens L1("
  "  thickness   = 2e-3,"
  "  focalLength = focalLength,"
  "  diameter    = D);"

  "on backFocalPlane of L1 Detector bfpDet;"
  "on imagePlane of L1 Detector imgDet;"
  "on objectPlane of L1 port object;"

  "path bfp L1 to bfpDet;"
  "path img L1 to imgDet;";


struct SpotStatistics {
  Real maxRad = 0.0;
  Real rmsRad = 0.0;
  
  Real x0     = 0.0;
  Real y0     = 0.0;

  Real fNum   = 0;

  void computeFromSurface(
    OpticalSurface const *fp,
    Vec3 const &chiefRay = -Vec3::eZ());
};


void
SpotStatistics::computeFromSurface(
  OpticalSurface const *fp,
  Vec3 const &chiefRay)
{
  Real R2     = 0;
  auto locations  = fp->locations();
  auto directions = fp->directions();
  size_t N     =  locations.size() / 3;

  const Vec3 *locVec = reinterpret_cast<const Vec3 *>(locations.data());
  const Vec3 *dirVec = reinterpret_cast<const Vec3 *>(directions.data());

  Vec3 center = sumPrecise<Vec3>(locVec, N);

  x0 = center.x / N;
  y0 = center.y / N;

  Real corr, c, t;

  for (size_t i = 3; i < locations.size(); i += 3) {
    Real x = locations[i] - x0;
    Real y = locations[i + 1] - y0;

    R2 = x * x + y * y;
    if (R2 > maxRad) {
      fNum = .5 / tan(acos(dirVec[i / 3] * chiefRay));
      maxRad = R2;
    }

    corr = R2 - c;
    t = rmsRad + corr;
    c = (t - rmsRad) - corr;
    rmsRad = t;
  }

  rmsRad = sqrt(rmsRad / static_cast<Real>(N));
  maxRad = sqrt(maxRad);
}

TEST_CASE("Parabolic reflector: center and focus", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_parabolicReflectorCode);
  REQUIRE(model);

  model->setDof("focalLength", 1);
  model->setDof("D", 1);

  auto m1 = model->lookupOpticalElement("M1");
  REQUIRE(m1);

  auto detector = model->lookupOpticalElement("det");
  REQUIRE(detector);

  auto surfaces = detector->opticalSurfaces();
  REQUIRE(surfaces.size() == 1);

  auto fp = surfaces.front();
  REQUIRE(fp != nullptr);

  // Record hits!
  detector->setRecordHits(true);

  std::list<Ray> rays;
  BeamProperties beamProp;

  beamProp.id              = 0;
  beamProp.length          = 1;
  beamProp.diameter        = 1;
  beamProp.offset          = Vec3::zero();
  beamProp.direction       = -Vec3::eZ();
  beamProp.angularDiameter = 0;
  beamProp.numRays         = 1000;
  beamProp.shape           = Circular;
  beamProp.objectShape     = PointLike;
  beamProp.random          = true;
  beamProp.setElementRelative(m1);
  beamProp.collimate();

  OMModel::addBeam(rays, beamProp);
  REQUIRE(rays.size() == beamProp.numRays);
  
  REQUIRE(model->traceDefault(rays));

  // Do statistics
  SpotStatistics statistics;
  statistics.computeFromSurface(fp);

  // Okay, things we need: that all rays have passed and that all of them
  // are perfectly focused
  REQUIRE(fp->hits.size() == rays.size());
  REQUIRE(isZero(statistics.x0));
  REQUIRE(isZero(statistics.y0));

  delete model;
}

TEST_CASE("Parabolic reflector: expected f/#", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_parabolicReflectorCode);
  REQUIRE(model);

  auto m1 = model->lookupOpticalElement("M1");
  REQUIRE(m1);

  auto vertex = m1->getPortFrame("vertex");
  REQUIRE(vertex);

  auto aperture = m1->getPortFrame("aperture");
  REQUIRE(aperture);

  auto detector = model->lookupOpticalElement("det");
  REQUIRE(detector);

  auto surfaces = detector->opticalSurfaces();
  REQUIRE(surfaces.size() == 1);

  auto fp = surfaces.front();
  REQUIRE(fp != nullptr);

  // Record hits!
  detector->setRecordHits(true);

  std::list<Ray> rays;
  BeamProperties beamProp;
  Real focalLength = 1;
  Real diameter = 1;

  beamProp.id              = 0;
  beamProp.length          = 1;
  beamProp.diameter        = diameter;
  beamProp.offset          = Vec3::zero();
  beamProp.direction       = -Vec3::eZ();
  beamProp.angularDiameter = 0;
  beamProp.numRays         = 100;
  beamProp.shape           = Ring;
  beamProp.objectShape     = PointLike;
  beamProp.random          = false;
  beamProp.setPlaneRelative(model->world());
  beamProp.collimate();

  OMModel::addBeam(rays, beamProp);
  REQUIRE(rays.size() == beamProp.numRays);
  
  model->setDof("focalLength", focalLength);

  model->setDof("D", diameter);
  Vec3 apertureLocation = aperture->getCenter();

  model->setDof("D", diameter + 1e-3); // Extra clearance to avoid edge rays
  
  REQUIRE(model->traceDefault(rays));

  // Do statistics
  SpotStatistics statistics;
  statistics.computeFromSurface(fp);

  // Now, in theory, the f/# should be f/D = 1/1 = 1. However, we are going to
  // get a smaller number. Why? Because the focal length is measured
  // from the aperture, but in a paraboloid the focal length refers to a 
  // distance from the vertex. We need to account for this difference by
  // taking into account the relative distance between vertex and aperture.

  Real dishDepth    = (apertureLocation - vertex->getCenter()).norm();  
  Real desiredFNum  = focalLength / diameter;
  Real expectedFNum = (focalLength - dishDepth) / diameter;
  
  printf("Desired  f/#: %g\n", desiredFNum);
  printf("Expected f/#: %g\n", expectedFNum);
  printf("Obtained f/#: %g\n", statistics.fNum);

  REQUIRE(expectedFNum == statistics.fNum);

  delete model;
}

TEST_CASE("Ideal lens: center and focus (infinity)", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_idealFocusLens);
  REQUIRE(model);

  auto L1 = model->lookupOpticalElement("L1");
  REQUIRE(L1);

  auto surfaces = L1->opticalSurfaces();
  REQUIRE(surfaces.size() == 1);

  auto iSurf = surfaces.front();
  REQUIRE(iSurf != nullptr);

  auto detector = model->lookupOpticalElement("bfpDet");
  REQUIRE(detector);

  surfaces = detector->opticalSurfaces();
  REQUIRE(surfaces.size() == 1);

  auto fp = surfaces.front();
  REQUIRE(fp != nullptr);

  // Record hits!
  L1->setRecordHits(true);
  detector->setRecordHits(true);

  std::list<Ray> rays;
  BeamProperties beamProp;
  Real focalLength = 0.2;
  Real diameter    = 0.05;

  beamProp.id              = 0;
  beamProp.length          = 1;
  beamProp.diameter        = diameter;
  beamProp.offset          = Vec3::zero();
  beamProp.direction       = -Vec3::eZ();
  beamProp.angularDiameter = 0;
  beamProp.numRays         = 100;
  beamProp.shape           = Ring;
  beamProp.objectShape     = PointLike;
  beamProp.random          = false;
  beamProp.setElementRelative(L1);
  beamProp.collimate();

  OMModel::addBeam(rays, beamProp);
  REQUIRE(rays.size() == beamProp.numRays);

  model->setDof("D", diameter + 1e-3); // Extra clearance to avoid edge rays
  model->setDof("focalLength", focalLength);

  REQUIRE(model->trace("bfp", rays));
  REQUIRE(iSurf->hits.size() == rays.size());

  REQUIRE(fp->hits.size() == rays.size());

  Real idealFNum = focalLength / diameter;

  // Do statistics
  SpotStatistics statistics;
  statistics.computeFromSurface(fp);
  printf("f/#: %g (ideal %g)\n", fabs(statistics.fNum), idealFNum);
  printf("MaxRadius: %g\n", statistics.maxRad);

  REQUIRE(fp->hits.size() == rays.size());
  REQUIRE(isZero(statistics.x0));
  REQUIRE(isZero(statistics.y0));
  REQUIRE(isZero(statistics.maxRad));
  REQUIRE(releq(fabs(statistics.fNum), idealFNum));

  REQUIRE(statistics.maxRad < 5e-4); // Make room for aberrations

  delete model;
}

TEST_CASE("Positive lens: center and focus (infinity)", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_focusLens);
  REQUIRE(model);

  auto L1 = model->lookupOpticalElement("L1");
  REQUIRE(L1);

  auto surfaces = L1->opticalSurfaces();
  REQUIRE(surfaces.size() == 2);

  auto iSurf = surfaces.front();
  REQUIRE(iSurf != nullptr);

  auto oSurf = surfaces.back();
  REQUIRE(oSurf != nullptr);

  auto detector = model->lookupOpticalElement("bfpDet");
  REQUIRE(detector);

  surfaces = detector->opticalSurfaces();
  REQUIRE(surfaces.size() == 1);

  auto fp = surfaces.front();
  REQUIRE(fp != nullptr);

  // Record hits!
  L1->setRecordHits(true);
  detector->setRecordHits(true);

  std::list<Ray> rays;
  BeamProperties beamProp;
  Real focalLength = 0.2;
  Real diameter    = 0.05;

  beamProp.id              = 0;
  beamProp.length          = 1;
  beamProp.diameter        = diameter;
  beamProp.offset          = Vec3::zero();
  beamProp.direction       = -Vec3::eZ();
  beamProp.angularDiameter = 0;
  beamProp.numRays         = 100;
  beamProp.shape           = Ring;
  beamProp.objectShape     = PointLike;
  beamProp.random          = false;
  beamProp.setElementRelative(L1);
  beamProp.collimate();

  OMModel::addBeam(rays, beamProp);
  REQUIRE(rays.size() == beamProp.numRays);

  model->setDof("K", -1);
  model->setDof("D", diameter + 1e-3); // Extra clearance to avoid edge rays
  model->setDof("focalLength", focalLength);

  REQUIRE(model->trace("bfp", rays));
  REQUIRE(iSurf->hits.size() == rays.size());
  REQUIRE(oSurf->hits.size() == rays.size());

  REQUIRE(fp->hits.size() == rays.size());

  Real idealFNum = focalLength / diameter;

  // Do statistics
  SpotStatistics statistics;
  statistics.computeFromSurface(fp);
  printf("f/#: %g (ideal %g)\n", fabs(statistics.fNum), idealFNum);

  REQUIRE(fp->hits.size() == rays.size());
  REQUIRE(isZero(statistics.x0));
  REQUIRE(isZero(statistics.y0));
  REQUIRE(statistics.maxRad < 5e-4); // Make room for aberrations

  delete model;
}
