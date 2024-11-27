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
#include <catch2/catch_test_macros.hpp>
#include <WorldFrame.h>
#include <Singleton.h>
#include <BenchElement.h>
#include <algorithm>

using namespace RZ;

TEST_CASE("Bench instantiation", "[libRZ]")
{
  WorldFrame world("world");
  BenchElement element(nullptr, "bench", &world);
}

TEST_CASE("Bench identification of ports", "[libRZ]")
{
  WorldFrame world("world");
  BenchElement element(nullptr, "bench", &world);

  auto ports = element.ports();
  REQUIRE(ports.find("surface") != ports.end());
}

TEST_CASE("Bench identification of properties", "[libRZ]")
{
  WorldFrame world("world");
  BenchElement element(nullptr, "bench", &world);

  auto props = element.properties();
  REQUIRE(props.find("height") != props.end());
}

TEST_CASE("Bench surface access", "[libRZ]")
{
  WorldFrame world("world");
  BenchElement element(nullptr, "bench", &world);
  ReferenceFrame *surf = element.getPortFrame("surface");

  REQUIRE(surf !=  nullptr);
}

TEST_CASE("Bench surface height access", "[libRZ]")
{
  WorldFrame world("world");
  BenchElement element(nullptr, "bench", &world);
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
  BenchElement element(nullptr, "bench", &world);
  BenchElement *newElement;
  Singleton *singleton = Singleton::instance();
  Point3 point(RZ_URANDSIGN, RZ_URANDSIGN, 1 + RZ_URANDSIGN);

  // Stack a  bench on top of the same bench
  newElement = element.plug<BenchElement>("surface", "BenchElement", "new_bench");
  REQUIRE(newElement);
  
  ReferenceFrame *surf = newElement->getPortFrame("surface");
  REQUIRE(surf != nullptr);

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
