#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <WorldFrame.h>
#include <Singleton.h>
#include <BenchElement.h>
#include <algorithm>

using namespace RZ;

TEST_CASE("Bench instantiation", "[libRZ]")
{
  WorldFrame world("world");
  BenchElement element("bench", &world);
}

TEST_CASE("Bench identification of ports", "[libRZ]")
{
  WorldFrame world("world");
  BenchElement element("bench", &world);

  auto ports = element.ports();
  REQUIRE(ports.find("surface") != ports.end());
}

TEST_CASE("Bench identification of properties", "[libRZ]")
{
  WorldFrame world("world");
  BenchElement element("bench", &world);

  auto props = element.properties();
  REQUIRE(props.find("height") != props.end());
}

TEST_CASE("Bench surface access", "[libRZ]")
{
  WorldFrame world("world");
  BenchElement element("bench", &world);
  ReferenceFrame *surf = element.getPortFrame("surface");

  REQUIRE(surf !=  nullptr);
}

TEST_CASE("Bench surface height access", "[libRZ]")
{
  WorldFrame world("world");
  BenchElement element("bench", &world);
  Point3 point(RZ_URANDSIGN, RZ_URANDSIGN, 1 + RZ_URANDSIGN);
  Real height = RZ_URANDSIGN + 1;

  ReferenceFrame *surf = element.getPortFrame("surface");
  REQUIRE(surf !=  nullptr);

  surf->addPoint("testPoint", point);
  world.recalculate();

  element.set("height", height);

  auto trueLocation = surf->getPoint("testPoint");
  REQUIRE(trueLocation != nullptr);

  REQUIRE(*trueLocation == point + height * Vec3::eZ());
}

TEST_CASE("Bench stacking", "[libRZ]")
{
  WorldFrame world("world");
  BenchElement element("bench", &world);
  BenchElement *newElement;
  Singleton *singleton = Singleton::instance();
  Point3 point(RZ_URANDSIGN, RZ_URANDSIGN, 1 + RZ_URANDSIGN);

  // Stack a  bench on top of the same bench
  newElement = element.plug<BenchElement>("surface", "Bench", "new_bench");
  ReferenceFrame *surf = newElement->getPortFrame("surface");

  surf->addPoint("testPoint", point);
  world.recalculate();

  for (auto i = 0; i < 100; ++i) {
    Real height1 = RZ_URANDSIGN + 1;
    Real height2 = RZ_URANDSIGN + 1;

    element.set("height", height1);
    newElement->set("height", height2);

    auto trueLocation = surf->getPoint("testPoint");
    REQUIRE(trueLocation != nullptr);
    REQUIRE(*trueLocation == point + (height1 + height2) * Vec3::eZ());
  }
}
