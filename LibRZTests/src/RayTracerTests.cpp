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
#define THIS_TEST_TAG "[RayTracer]"

#include <catch2/catch_test_macros.hpp>
#include <CPURayTracingEngine.h>
#include <Singleton.h>
#include <WorldFrame.h>
#include <RotatedFrame.h>
#include <cstdlib>
#include <iostream>
#include <OpticalElement.h>

#define BEAM_SIZE 100

using namespace RZ;

TEST_CASE("Raytracer instantiation", THIS_TEST_TAG)
{
  CPURayTracingEngine engine;
}

TEST_CASE("Pushing rays to raytracer", THIS_TEST_TAG)
{
  CPURayTracingEngine engine;

  engine.pushRay(Point3::zero(), Vec3(1, 1, 1));
  engine.pushRay(Point3::zero(), Vec3(1, 1, 2));
  engine.pushRay(Point3::zero(), Vec3(3, 1, 0));
}

TEST_CASE("Ensuring plane intercept works for canonical cases", THIS_TEST_TAG)
{
  RayTransferProcessor *processor = Singleton::instance()->lookupRayTransferProcessor("PassThrough");
  WorldFrame world("world");
  OpticalSurface surf;

  REQUIRE(processor != nullptr);

  surf.frame = &world;
  surf.processor = processor;

  for (auto i = 0; i < 100; ++i) {
    CPURayTracingEngine engine;

    world.recalculate();

    // Define the plane normal
    Vec3   normal  = world.eZ();
    Point3 origin  = Vec3::zero();
    Real   dist    = 10;
    Point3 source  = origin + dist * normal;
    Real   radius  = 10 * (1 + RZ_URANDSIGN);
    
    for (auto j = 0; j < BEAM_SIZE; ++j) {
      Real angle          = RZ_URANDSIGN * M_PI;
      Vec3 planeDirection = cos(angle) * world.eX() + sin(angle) * world.eY();
      Vec3 direction      = (radius * planeDirection - dist * normal).normalized();
      engine.pushRay(source, direction);
    }

    engine.setCurrentSurface(&surf);
    engine.trace();
    REQUIRE(engine.beam()->count == BEAM_SIZE);
    
    engine.transfer();

    auto outputRays = engine.getRays();
    REQUIRE(outputRays.size() == BEAM_SIZE);

    for (auto ray = outputRays.begin(); ray != outputRays.end(); ++ray) {
      auto relRay = ray->origin + ray->length * ray->direction - origin;

      REQUIRE(ray->length >= dist);
      REQUIRE(isZero(relRay * normal));
      REQUIRE(releq(relRay.norm(), radius));
    }
  }
}

TEST_CASE("Ensuring that all rays are intercepted in the destination plane", THIS_TEST_TAG)
{
  RayTransferProcessor *processor = Singleton::instance()->lookupRayTransferProcessor("PassThrough");
  WorldFrame world("world");
  RotatedFrame frame("detector", &world, Vec3::eZ(), 0);
  OpticalSurface surf;

  REQUIRE(processor != nullptr);

  surf.frame = &frame;
  surf.processor = processor;

  for (auto i = 0; i < 100; ++i) {
    CPURayTracingEngine engine;

    frame.setRotation(Vec3(RZ_URANDSIGN, RZ_URANDSIGN, RZ_URANDSIGN), RZ_URANDSIGN * M_PI);
    world.recalculate();

    // Define the plane normal
    Vec3   normal  = frame.eZ();
    Point3 origin  = frame.getCenter();
    Real   dist    = .1 + 10 * .5 * (RZ_URANDSIGN + 1);
    Point3 source  = origin + dist * normal;
    Real   radius  = 10 * (1 + RZ_URANDSIGN);
    
    for (auto j = 0; j < BEAM_SIZE; ++j) {
      Real angle          = RZ_URANDSIGN * M_PI;
      Vec3 planeDirection = cos(angle) * frame.eX() + sin(angle) * frame.eY();
      Vec3 direction      = radius * planeDirection - dist * normal;

      engine.pushRay(source, direction.normalized());
    }

    engine.setCurrentSurface(&surf);
    engine.trace();
    REQUIRE(engine.beam()->count == BEAM_SIZE);
    
    engine.transfer();

    auto outputRays = engine.getRays();
    REQUIRE(outputRays.size() == BEAM_SIZE);

    for (auto ray = outputRays.begin(); ray != outputRays.end(); ++ray) {
      auto relRay = ray->origin + ray->length * ray->direction - origin;

      REQUIRE(ray->length >= dist);
      REQUIRE(isZero(relRay * normal));
      REQUIRE(releq(relRay.norm(), radius));
    }
  }
}

