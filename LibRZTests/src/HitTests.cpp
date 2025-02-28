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

  "on backFocalPlane of L1 Detector bfpDet(flip = true);"
  "on imagePlane of L1 Detector imgDet(flip = true);"
  "on objectPlane of L1 port object;"

  "path bfp L1 to bfpDet;"
  "path img L1 to imgDet;";

static const char *g_idealFocusLens = 
  "dof focalLength(.1, .3) = .2;"
  "dof D = 5e-2;"

  "IdealLens L1("
  "  focalLength = focalLength,"
  "  diameter    = D);"

  "on backFocalPlane of L1 Detector bfpDet(flip = true);"
  "on imagePlane of L1 Detector imgDet(flip = true);"
  "on objectPlane of L1 port object;"

  "path bfp L1 to bfpDet;"
  "path img L1 to imgDet;";

static const char *g_asymetricLens = 
  "dof Kf(-4, 4) = -1;"
  "dof Kb(-4, 4) = -1;"
  
  "dof frontFocalLength(.1, .5) = .2;"
  "dof backFocalLength(.1, .5)  = .2;"

  "dof D = 5e-2;"

  "ConicLens L1("
  "  thickness        = 2e-3,"
  "  frontConic       = Kf,"
  "  backConic        = Kb,"
  "  frontFocalLength = frontFocalLength,"
  "  backFocalLength  = backFocalLength,"
  "  diameter         = D);"

  "on backFocalPlane of L1 Detector bfpDet(flip = true);"
  "on imagePlane of L1 Detector imgDet(flip = true);"
  "on objectPlane of L1 port object;"

  "path bfp L1 to bfpDet;"
  "path img L1 to imgDet;";


struct BeamTestStatistics {
  Real maxRad = 0.0;
  Real rmsRad = 0.0;
  
  Real x0     = 0.0;
  Real y0     = 0.0;

  Real fNum   = std::numeric_limits<Real>::infinity();

  uint64_t intercepted = 0;
  uint64_t vignetted   = 0;
  uint64_t pruned      = 0;

  void computeFromSurface(
    OpticalSurface const *fp,
    Vec3 const &chiefRay = -Vec3::eZ());

  void computeFromRayList(
    std::list<Ray> const &rays,
    Vec3 const &chiefRay = -Vec3::eZ());
};

void
BeamTestStatistics::computeFromSurface(
  OpticalSurface const *fp,
  Vec3 const &chiefRay)
{
  Real R2         = 0;
  auto locations  = fp->locations();
  auto directions = fp->directions();
  size_t N        = locations.size() / 3;

  const Vec3 *locVec = reinterpret_cast<const Vec3 *>(locations.data());
  const Vec3 *dirVec = reinterpret_cast<const Vec3 *>(directions.data());

  Vec3 center = sumPrecise<Vec3>(locVec, N);

  x0 = center.x / N;
  y0 = center.y / N;

  Real corr = 0, c = 0, t;
  Real currFnum;

  
  for (size_t i = 3; i < locations.size(); i += 3) {
    Real x = locations[i] - x0;
    Real y = locations[i + 1] - y0;

    R2     = x * x + y * y;
    maxRad = fmax(R2, maxRad);
    fNum   = fabsmin(.5 / tan(acos(dirVec[i / 3] * chiefRay)), fNum);

    corr   = R2 - c;
    t      = rmsRad + corr;
    c      = (t - rmsRad) - corr;
    rmsRad = t;
  }

  rmsRad = sqrt(rmsRad / static_cast<Real>(N));
  maxRad = sqrt(maxRad);

  vignetted = intercepted = pruned = 0;

  for (auto &p : fp->statistics) {
    intercepted += p.second.intercepted;
    vignetted   += p.second.vignetted;
    pruned      += p.second.pruned;
  }
}

void
BeamTestStatistics::computeFromRayList(
  std::list<Ray> const &rays,
  Vec3 const &chiefRay)
{
  Real  R2     = 0;
  size_t N     =  rays.size();

  Vec3 vCorr, vC, vT;
  Vec3 center;

  for (auto const &ray : rays) {
    vCorr  = ray.origin - vC;
    vT     = center + vCorr;
    vC     = (vT - center) - vCorr;
    center = vT;
  }

  x0 = center.x / N;
  y0 = center.y / N;

  Real corr = 0, c = 0, t;

  for (auto const &ray : rays) {
    Real x = ray.origin.x - x0;
    Real y = ray.origin.y - y0;

    R2     = x * x + y * y;
    maxRad = fmax(R2, maxRad);
    fNum   = fabsmin(.5 / tan(acos(ray.direction * chiefRay)), fNum);

    corr   = R2 - c;
    t      = rmsRad + corr;
    c      = (t - rmsRad) - corr;
    rmsRad = t;
  }

  rmsRad = sqrt(rmsRad / static_cast<Real>(N));
  maxRad = sqrt(maxRad);
}

TEST_CASE("Parabolic reflector: center and focus", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_parabolicReflectorCode);
  REQUIRE(model);

  REQUIRE(model->setDof("focalLength", 1));
  REQUIRE(model->setDof("D", 1));

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
  BeamTestStatistics statistics;
  statistics.computeFromSurface(fp);

  // Okay, things we need: that all rays have passed and that all of them
  // are perfectly focused

  REQUIRE(statistics.pruned      == 0);
  REQUIRE(statistics.vignetted   == 0);
  REQUIRE(statistics.intercepted == rays.size());
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
  
  REQUIRE(model->setDof("focalLength", focalLength));

  REQUIRE(model->setDof("D", diameter));
  Vec3 apertureLocation = aperture->getCenter();

  REQUIRE(model->setDof("D", diameter + 1e-3)); // Extra clearance to avoid edge rays
  
  REQUIRE(model->traceDefault(rays));

  // Do statistics
  BeamTestStatistics statistics;

  statistics.computeFromSurface(fp);

  // Now, in theory, the f/# should be f/D = 1/1 = 1. However, we are going to
  // get a smaller number. Why? Because the focal length is measured
  // from the aperture, but in a paraboloid the focal length refers to a 
  // distance from the vertex. We need to account for this difference by
  // taking into account the relative distance between vertex and aperture.

  Real dishDepth    = (apertureLocation - vertex->getCenter()).norm();  
  Real desiredFNum  = focalLength / diameter;
  Real expectedFNum = (focalLength - dishDepth) / diameter;
  
  printf("(inf) Parabolic mirror: Desired  f/#: %g\n", desiredFNum);
  printf("(inf) Parabolic mirror: Expected f/#: %g\n", expectedFNum);
  printf("(inf) Parabolic mirror: Obtained f/#: %g\n", statistics.fNum);

  REQUIRE(releq(expectedFNum, statistics.fNum));

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
  beamProp.numRays         = 1000;
  beamProp.shape           = Circular;
  beamProp.objectShape     = PointLike;
  beamProp.random          = true;
  beamProp.setElementRelative(L1);
  beamProp.collimate();

  OMModel::addBeam(rays, beamProp);
  REQUIRE(rays.size() == 1000);

  REQUIRE(model->setDof("D", diameter + 1e-3)); // Extra clearance to avoid edge rays
  REQUIRE(model->setDof("focalLength", focalLength));

  REQUIRE(model->trace("bfp", rays));
  REQUIRE(iSurf->hits.size() == rays.size());

  REQUIRE(fp->hits.size() == rays.size());

  Real idealFNum = focalLength / diameter;

  BeamTestStatistics statistics;
  statistics.computeFromSurface(fp);

  printf("(inf) Ideal lens: f/#: %g (>= ideal %g)\n", fabs(statistics.fNum), idealFNum);
  printf("(inf) Ideal lens: MaxRadius: %g\n", statistics.maxRad);

  REQUIRE(fp->hits.size() == rays.size());
  REQUIRE(isZero(statistics.x0));
  REQUIRE(isZero(statistics.y0));
  REQUIRE(isZero(statistics.maxRad));
  REQUIRE(fabs(statistics.fNum) >= idealFNum);

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

  REQUIRE(model->setDof("K", -1));
  REQUIRE(model->setDof("D", diameter + 1e-3)); // Extra clearance to avoid edge rays
  REQUIRE(model->setDof("focalLength", focalLength));

  REQUIRE(model->trace("bfp", rays));
  REQUIRE(iSurf->hits.size() == rays.size());
  REQUIRE(oSurf->hits.size() == rays.size());

  REQUIRE(fp->hits.size() == rays.size());

  Real idealFNum = focalLength / diameter;

  BeamTestStatistics statistics;
  statistics.computeFromSurface(fp);

  printf("(inf) Positive lens: f/#: %g (ideal %g)\n", fabs(statistics.fNum), idealFNum);
  printf("(inf) Positive lens: MaxRadius: %g\n", statistics.maxRad);

  REQUIRE(fp->hits.size() == rays.size());
  REQUIRE(isZero(statistics.x0));
  REQUIRE(isZero(statistics.y0));
  REQUIRE(statistics.maxRad < 5e-4); // Make room for aberrations

  delete model;
}

TEST_CASE("Ideal lens: center and focus (object)", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_idealFocusLens);
  REQUIRE(model);

  auto L1 = model->lookupOpticalElement("L1");
  REQUIRE(L1);

  auto surfaces = L1->opticalSurfaces();
  REQUIRE(surfaces.size() == 1);

  auto iSurf = surfaces.front();
  REQUIRE(iSurf != nullptr);

  auto detector = model->lookupOpticalElement("imgDet");
  REQUIRE(detector);

  surfaces = detector->opticalSurfaces();
  REQUIRE(surfaces.size() == 1);

  auto fp = surfaces.front();
  REQUIRE(fp != nullptr);

  auto object = model->lookupReferenceFrame("object");
  REQUIRE(object != nullptr);

  // Record hits!
  L1->setRecordHits(true);
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
 
  printf("(obj) Ideal lens: aperture angle: %g deg\n", rad2deg(beamProp.angularDiameter));

  OMModel::addBeam(rays, beamProp);
  REQUIRE(rays.size() == 1000);

  BeamTestStatistics inputStatistics;
  inputStatistics.computeFromRayList(rays, beamProp.direction);

  printf("(obj) Ideal lens: object radius: %g\n", inputStatistics.maxRad);
  printf("(obj) Ideal lens: object center: %g, %g\n", inputStatistics.x0, inputStatistics.y0);
  
  printf(
    "(obj) Ideal lens: object f/#: %g (err = %g)\n",
    inputStatistics.fNum,
    inputStatistics.fNum - idealFNum);
  REQUIRE(releq(inputStatistics.fNum, idealFNum));
  REQUIRE(isZero(inputStatistics.x0));
  REQUIRE(isZero(inputStatistics.y0));
  
  // Add all other rays
  beamProp.numRays     = 100;
  beamProp.objectShape = CircleLike;
  beamProp.random      = true;
  
  OMModel::addBeam(rays, beamProp);
  REQUIRE(rays.size() == 1100);

  REQUIRE(model->setDof("D", diameter + 1e-3)); // Extra clearance to avoid edge rays
  REQUIRE(model->setDof("focalLength", focalLength));

  REQUIRE(model->trace("img", rays));
  REQUIRE(iSurf->hits.size() == rays.size());

  REQUIRE(fp->hits.size() == rays.size());


  // Do statistics on the image
  BeamTestStatistics statistics;
  statistics.computeFromSurface(fp);
  printf("(obj) Ideal lens: image radius: %g\n", statistics.maxRad);
  printf("(obj) Ideal lens: image center: %g, %g\n", statistics.x0, statistics.y0);
  printf("(obj) Ideal lens: image f/#: %g (err = %g)\n", statistics.fNum, statistics.fNum - idealFNum);
  
  REQUIRE(fp->hits.size() == rays.size());
  REQUIRE(isZero(statistics.x0));
  REQUIRE(isZero(statistics.y0));
  REQUIRE(isZero(statistics.maxRad));
  REQUIRE(releq(statistics.fNum, idealFNum));

  delete model;
}

TEST_CASE("Positive lens: center and focus (object)", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_focusLens);
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
 
  printf("(obj) Positive lens: aperture angle: %g deg\n", rad2deg(beamProp.angularDiameter));

  OMModel::addBeam(rays, beamProp);
  REQUIRE(rays.size() == 1000);

  BeamTestStatistics inputStatistics;
  inputStatistics.computeFromRayList(rays, beamProp.direction);
  printf("(obj) Positive lens: object radius: %g\n", inputStatistics.maxRad);
  printf("(obj) Positive lens: object center: %g, %g\n", inputStatistics.x0, inputStatistics.y0);
  
  printf(
    "(obj) Positive lens: object f/#: %g (err = %g)\n",
    inputStatistics.fNum,
    inputStatistics.fNum - idealFNum);
  REQUIRE(releq(inputStatistics.fNum, idealFNum));
  REQUIRE(isZero(inputStatistics.x0));
  REQUIRE(isZero(inputStatistics.y0));
  
  REQUIRE(model->setDof("D", diameter + 1e-3)); // Extra clearance to avoid edge rays
  REQUIRE(model->setDof("focalLength", focalLength));

  REQUIRE(model->trace("img", rays));
  REQUIRE(fp->hits.size() == rays.size());

  // Do statistics on the image
  BeamTestStatistics statistics;
  statistics.computeFromSurface(fp);
  printf("(obj) Positive lens: image radius: %g\n", statistics.maxRad);
  printf("(obj) Positive lens: image center: %g, %g\n", statistics.x0, statistics.y0);
  printf("(obj) Positive lens: image f/#: %g (ideal %g)\n", statistics.fNum, idealFNum);
  
  // Require f/# within 2% error
  REQUIRE(fp->hits.size() == rays.size());
  REQUIRE(isZero(statistics.x0));
  REQUIRE(isZero(statistics.y0));
  REQUIRE(statistics.maxRad < 3e-4); // Room for aberrations
  REQUIRE(releq(statistics.fNum, idealFNum, 2e-2));
  
  delete model;
}

TEST_CASE("Asymmetric lens: center and focus (object)", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(g_asymetricLens);
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
 
  printf("(obj) Asymmetric lens: aperture angle: %g deg\n", rad2deg(beamProp.angularDiameter));

  OMModel::addBeam(rays, beamProp);
  REQUIRE(rays.size() == 1000);

  BeamTestStatistics inputStatistics;
  inputStatistics.computeFromRayList(rays, beamProp.direction);
  printf("(obj) Asymmetric lens: object radius: %g\n", inputStatistics.maxRad);
  printf("(obj) Asymmetric lens: object center: %g, %g\n", inputStatistics.x0, inputStatistics.y0);
  
  printf(
    "(obj) Asymmetric lens: object f/#: %g (err = %g)\n",
    inputStatistics.fNum,
    inputStatistics.fNum - idealFNum);
  REQUIRE(releq(inputStatistics.fNum, idealFNum));
  REQUIRE(isZero(inputStatistics.x0));
  REQUIRE(isZero(inputStatistics.y0));
  
  REQUIRE(model->setDof("D", diameter + 1e-3)); // Extra clearance to avoid edge rays
  REQUIRE(model->setDof("frontFocalLength", focalLength));
  REQUIRE(model->setDof("backFocalLength", 2 * focalLength));

  REQUIRE((Real) L1->get("frontFocalLength") == focalLength);
  REQUIRE((Real) L1->get("backFocalLength") == 2 * focalLength);
  
  REQUIRE(model->trace("img", rays));
  REQUIRE(fp->hits.size() == rays.size());

  // Do statistics on the image
  BeamTestStatistics statistics;
  statistics.computeFromSurface(fp);
  printf("(obj) Asymmetric lens: image radius: %g\n", statistics.maxRad);
  printf("(obj) Asymmetric lens: image center: %g, %g\n", statistics.x0, statistics.y0);
  printf("(obj) Asymmetric lens: image f/#: %g (ideal %g)\n", statistics.fNum, 2 * idealFNum);
  
  REQUIRE(fp->hits.size() == rays.size());
  REQUIRE(isZero(statistics.x0));
  REQUIRE(isZero(statistics.y0));
  REQUIRE(statistics.maxRad < 4e-4); // Room for aberrations
  REQUIRE(releq(statistics.fNum, 2 * idealFNum, 2e-2));

  delete model;
}
