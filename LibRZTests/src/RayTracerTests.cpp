#define CATCH_CONFIG_MAIN
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

TEST_CASE("Raytracer instantiation", "[libRZ]")
{
  CPURayTracingEngine engine;
}

TEST_CASE("Pushing rays to raytracer", "[libRZ]")
{
  CPURayTracingEngine engine;

  engine.pushRay(Point3::zero(), Vec3(1, 1, 1));
  engine.pushRay(Point3::zero(), Vec3(1, 1, 2));
  engine.pushRay(Point3::zero(), Vec3(3, 1, 0));
}

TEST_CASE("Ensuring plane intercept works for canonical cases", "[libRZ]")
{
  RayTransferProcessor *processor = Singleton::instance()->lookupRayTransferProcessor("PassThru");
  WorldFrame world("world");
  OpticalSurface surf;

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
      auto relRay = ray->origin - origin;

      REQUIRE(ray->length >= dist);
      REQUIRE(isZero(relRay * normal));
      REQUIRE(releq(relRay.norm(), radius));
    }
  }
}

TEST_CASE("Ensuring that all rays are intercepted in the destination plane", "[libRZ]")
{
  RayTransferProcessor *processor = Singleton::instance()->lookupRayTransferProcessor("PassThru");
  WorldFrame world("world");
  RotatedFrame frame("detector", &world, Vec3::eZ(), 0);
  OpticalSurface surf;

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
      auto relRay = ray->origin - origin;

      REQUIRE(ray->length >= dist);
      REQUIRE(isZero(relRay * normal));
      REQUIRE(releq(relRay.norm(), radius));
    }
  }
}

