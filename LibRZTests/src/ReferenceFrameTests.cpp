#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <ReferenceFrame.h>
#include <WorldFrame.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <cstdlib>
#include <iostream>

using namespace RZ;

TEST_CASE("Testing vector comparison", "[libRZ]")
{
  REQUIRE(Vec3::zero() == Vec3(0, 0, 0));
  REQUIRE(Vec3::zero() != Vec3::eX());
  REQUIRE(Vec3::eX() != Vec3::eY());
  REQUIRE(Vec3::eY() != Vec3::eZ());

  REQUIRE(Vec3(1, 2, 3) == Vec3(1, 2,  3));
  REQUIRE(Vec3(1, 2, 3) != Vec3(1, 2, -3));
}

TEST_CASE("Testing basic vector algebra", "[libRZ]")
{
  Vec3 sum = Vec3::eX() + 2 * Vec3::eY() - 3 * Vec3::eZ();

  // Test sums
  REQUIRE(sum == Vec3(1, 2, -3));
  REQUIRE(sum - Vec3(1, 2, -3) == Vec3::zero());
}

TEST_CASE("Testing cross products", "[libRZ]")
{
  REQUIRE(Vec3::eX().cross(Vec3::eY()) == Vec3::eZ());
  REQUIRE(Vec3::eY().cross(Vec3::eZ()) == Vec3::eX());
  REQUIRE(Vec3::eZ().cross(Vec3::eX()) == Vec3::eY());

  REQUIRE(Vec3::eY().cross(Vec3::eX()) == -Vec3::eZ());
  REQUIRE(Vec3::eZ().cross(Vec3::eY()) == -Vec3::eX());
  REQUIRE(Vec3::eX().cross(Vec3::eZ()) == -Vec3::eY());
}

TEST_CASE("Testing basic matrix algebra", "[libRZ]")
{
  Matrix3 eye = Matrix3::eye();
  Real x, y, z;

  unsigned int i;

  for (i = 0; i < 10000; ++i) {
    x = 2 * (static_cast<Real>(rand()) / RAND_MAX - .5);
    y = 2 * (static_cast<Real>(rand()) / RAND_MAX - .5);
    z = 2 * (static_cast<Real>(rand()) / RAND_MAX - .5);

    Vec3 vec(x, y, z);

    REQUIRE(eye * vec == vec);
  }
}

TEST_CASE("Testing composed rotations", "[libRZ]")
{
  Real x, y, z;

  unsigned int i;

  for (i = 0; i < 1000; ++i) {
    Real angle1 = 2 * M_PI * (static_cast<Real>(rand()) / RAND_MAX - .5);
    Real angle2 = 2 * M_PI * (static_cast<Real>(rand()) / RAND_MAX - .5);
    Real angle  = angle1 + angle2;
    x = 2 * (static_cast<Real>(rand()) / RAND_MAX - .5);
    y = 2 * (static_cast<Real>(rand()) / RAND_MAX - .5);
    z = 2 * (static_cast<Real>(rand()) / RAND_MAX - .5);

    Vec3 axis(x, y, z);

    axis = axis.normalized();

    Matrix3 R1 = Matrix3::rot(axis, angle1);
    Matrix3 R2 = Matrix3::rot(axis, angle2);
    Matrix3 R  = Matrix3::rot(axis, angle);

    REQUIRE(R1 * R2 == R);
  }
}

TEST_CASE("Testing matrix-vector poducts", "[libRZ]")
{
  Real x, y, z;
  Matrix3 M(Vec3::eY(), Vec3::eZ(), Vec3::eX());

  unsigned int i;

  for (i = 0; i < 1000; ++i) {
    x = 2 * (static_cast<Real>(rand()) / RAND_MAX - .5);
    y = 2 * (static_cast<Real>(rand()) / RAND_MAX - .5);
    z = 2 * (static_cast<Real>(rand()) / RAND_MAX - .5);

    Vec3 v(x, y, z);
    Vec3 expected(y, z, x);
  
    REQUIRE(M * v == expected);
  }
}


TEST_CASE("Testing rotation around different axes", "[libRZ]")
{
  Real x, y, z, angle;

  unsigned int i;

  for (i = 0; i < 10000; ++i) {
    angle = 2 * M_PI * (static_cast<Real>(rand()) / RAND_MAX - .5);
    x = 2 * (static_cast<Real>(rand()) / RAND_MAX - .5);
    y = 2 * (static_cast<Real>(rand()) / RAND_MAX - .5);
    z = 2 * (static_cast<Real>(rand()) / RAND_MAX - .5);

    angle = M_PI / 2;
    Vec3 vec(x, y, z);
    Vec3 rotVec(
      cos(angle) * x - sin(angle) * y, 
      sin(angle) * x + cos(angle) * y,
      z);
    Matrix3 R = Matrix3::rot(Vec3::eZ(), angle);
    Vec3 rotated = R * vec;

    REQUIRE(rotated == rotVec);
  }
}

TEST_CASE("Reference frame (instantiation)", "[libRZ]")
{
  WorldFrame wf("world");

  wf.recalculate();

  REQUIRE(wf.getCenter() == Vec3::zero());
  REQUIRE(wf.getOrientation() == Matrix3::eye());
}

TEST_CASE("Translated frame (instantiation)", "[libRZ]")
{
  WorldFrame world("world");
  Vec3 center(1, 2, 3);
  TranslatedFrame frame("translation", &world, center);

  world.recalculate();
  REQUIRE(frame.isCalculated());
}

TEST_CASE("Translated frame (verification)", "[libRZ]")
{
  WorldFrame world("world");
  Vec3 center(1, 2, 3);
  TranslatedFrame frame("translation", &world, center);

  world.recalculate();

  REQUIRE(world.getCenter() == Vec3::zero());
  REQUIRE(world.getOrientation() == Matrix3::eye());

  REQUIRE(frame.getCenter() == center);
  REQUIRE(frame.getOrientation() == Matrix3::eye());
}

TEST_CASE("Translated frame (definition of primitives)", "[libRZ]")
{
  WorldFrame world("world");
  Vec3 center(1, 2, 3);
  TranslatedFrame frame("translation", &world, center);
  Vec3 axis(0, 1, 2);
  Vec3 point(3, 4, 5);
  int index;
  const Vec3 *pAxis, *pPoint;

  index = frame.addAxis("axis", axis);
  REQUIRE(index != -1);
  REQUIRE(index == frame.getAxisIndex("axis"));

  index = frame.addPoint("point", point);
  REQUIRE(index != -1);
  REQUIRE(index == frame.getPointIndex("point"));

  world.recalculate();

  REQUIRE((pAxis = frame.getAxis("axis")) != nullptr);
  REQUIRE((pPoint = frame.getPoint("point")) != nullptr);

  std::cout << *pPoint << std::endl;

  REQUIRE(*pAxis == axis);
  REQUIRE(*pPoint == center + point);
}

TEST_CASE("Rotated frame (instantiation)", "[libRZ]")
{
  WorldFrame world("world");
  Vec3 axis(1, 1, 1);
  Real angle = deg2rad(45);
  RotatedFrame frame("rotation", &world, axis, angle);

  printf("Recalculating... frame is %p\n", &frame);

  
  world.recalculate();
  REQUIRE(frame.isCalculated());
}

TEST_CASE("Rotated frame (verification)", "[libRZ]")
{
  WorldFrame world("world");
  Real angle = deg2rad(45);
  Vec3 rotAxis(1, 1, 1);
  RotatedFrame frame("rotation", &world, rotAxis, angle);
  Matrix3 rotMatrix = Matrix3::rot(rotAxis.normalized(), angle);

  world.recalculate();

  REQUIRE(world.getCenter() == Vec3::zero());
  REQUIRE(world.getOrientation() == Matrix3::eye());

  REQUIRE(frame.getCenter() == world.getCenter());
  REQUIRE(frame.getOrientation() == rotMatrix);
}

TEST_CASE("Rotated frame (definition of primitives)", "[libRZ]")
{
  WorldFrame world("world");
  Vec3 rotAxis(1, 1, 1);
  Real angle = deg2rad(45);
  RotatedFrame frame("rotation", &world, rotAxis, angle);
  Matrix3 rotMatrix = Matrix3::rot(rotAxis.normalized(), angle);
  Vec3 axis(0, 1, 2);
  Vec3 point(3, 4, 5);
  int index;
  const Vec3 *pAxis, *pPoint;

  index = frame.addAxis("axis", axis);
  REQUIRE(index != -1);
  REQUIRE(index == frame.getAxisIndex("axis"));

  index = frame.addPoint("point", point);
  REQUIRE(index != -1);
  REQUIRE(index == frame.getPointIndex("point"));

  world.recalculate();

  REQUIRE((pAxis = frame.getAxis("axis")) != nullptr);
  REQUIRE((pPoint = frame.getPoint("point")) != nullptr);

  std::cout << *pPoint << std::endl;

  REQUIRE(*pAxis == rotMatrix * axis);
  REQUIRE(*pPoint == rotMatrix * point + frame.getCenter());
}

TEST_CASE("Frame composition", "[libRZ]")
{
  WorldFrame world("world");
  Vec3 rotAxis(1, 1, 1);
  Real angle = deg2rad(45);
  TranslatedFrame translatedFrame("translation", &world, Vec3::eY());
  RotatedFrame frame("rotation", &translatedFrame, rotAxis, angle);
  Matrix3 rotMatrix = Matrix3::rot(rotAxis.normalized(), angle);
  Vec3 axis(0, 1, 2);
  Vec3 point(1, 3, 9);
  int index;
  const Vec3 *pAxis, *pPoint;

  index = frame.addAxis("axis", axis);
  REQUIRE(index != -1);
  REQUIRE(index == frame.getAxisIndex("axis"));

  index = frame.addPoint("point", point);
  REQUIRE(index != -1);
  REQUIRE(index == frame.getPointIndex("point"));

  world.recalculate();

  REQUIRE((pAxis = frame.getAxis("axis")) != nullptr);
  REQUIRE((pPoint = frame.getPoint("point")) != nullptr);

  std::cout << *pPoint << std::endl;

  REQUIRE(*pAxis == rotMatrix * axis);
  REQUIRE(*pPoint == rotMatrix * point + Vec3::eY());
}

