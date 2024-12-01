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
#define THIS_TEST_TAG "[Element]"

#include <catch2/catch_test_macros.hpp>
#include <Element.h>
#include <WorldFrame.h>
#include <Singleton.h>
#include <algorithm>

using namespace RZ;

TEST_CASE("Element instantiation", THIS_TEST_TAG)
{
  WorldFrame world("world");

  for (auto &p : Singleton::instance()->elementFactories()) {
    auto factory = Singleton::instance()->lookupElementFactory(p);
    REQUIRE(factory != nullptr);

    auto element = factory->make(p, &world, nullptr);
    REQUIRE(element != nullptr);

    delete element;
  }
}

static inline bool
shouldSkip(std::string const &factory, std::string const &prop)
{
  if (prop == "conic" || prop == "frontConic" || prop == "backConic") {
    if (factory == "ConicMirror" || factory == "ConicLens")
      return false;
    else
      return true;
  }

  return false;
}

TEST_CASE("Element property access", THIS_TEST_TAG)
{
  WorldFrame world("world");
  
  for (auto &p : Singleton::instance()->elementFactories()) {
    auto factory = Singleton::instance()->lookupElementFactory(p);
    REQUIRE(factory != nullptr);

    auto element = factory->make(p, &world, nullptr);
    REQUIRE(element != nullptr);

    printf("Checking properties of %s:\n", p.c_str());

    for (auto &prop : element->properties()) {
      PropertyValue val = element->get(prop);
      printf("  - %10s [type = %d] ", prop.c_str(), val.type());
      REQUIRE(val.type() != UndefinedValue);

      Real prevReal, asReal, newReal;
      std::string prevString, asString, newString;
      int64_t prevInteger, asInteger, newInteger;
      bool prevBool, asBool, newBool;

      if (element->propertyIsHidden(prop)) {
        printf("(hidden)\n");
      } else switch (val.type()) {
        case RealValue:
          if (shouldSkip(p, prop)) {
            printf("(skipped)\n");
          } else {
            asReal = prevReal = std::get<Real>(val);
            if (isZero(asReal))
              asReal += 1e-1;
            else
              asReal *= 0.9;

            REQUIRE(element->set(prop, asReal));
            newReal = element->get(prop);
            printf("(%g -> %g)\n", asReal, newReal);
            REQUIRE(releq(asReal, newReal));
            REQUIRE(element->set(prop, prevReal));
          }
          
          break;

        case IntegerValue:
          asInteger = prevInteger = std::get<int64_t>(val);
          ++asInteger;
          REQUIRE(element->set(prop, asInteger));
          newInteger = element->get(prop);
          printf("%d -> %d\n", asInteger, newInteger);
          REQUIRE(asInteger == newInteger);
          REQUIRE(element->set(prop, prevInteger));
          break;

        case BooleanValue:
          // The optical property is read-only and should not be changed
          asBool = prevBool = std::get<bool>(val);
          asBool = !asBool;

          REQUIRE(element->set(prop, asBool));

          newBool = std::get<bool>(element->get(prop));
          printf("%d -> %d\n", asBool, newBool);

          REQUIRE(newBool == asBool);
          REQUIRE(element->set(prop, prevBool));
          break;

        case StringValue:
          asString = prevString = std::get<std::string>(val);
          asString += "-suffix";
          REQUIRE(element->set(prop, asString));
          newString = std::get<std::string>(element->get(prop));
          printf("(\"%s\" -> \"%s\")\n", asString.c_str(), newString.c_str());
          REQUIRE(newString == asString);
          REQUIRE(element->set(prop, prevString));
          break;
      }
    }

    delete element;
  }
}

TEST_CASE("Well known property tests", THIS_TEST_TAG)
{
  WorldFrame world("world");
  
  for (auto &p : Singleton::instance()->elementFactories()) {
    auto factory = Singleton::instance()->lookupElementFactory(p);
    REQUIRE(factory != nullptr);

    auto element = factory->make(p, &world, nullptr);
    REQUIRE(element != nullptr);

    printf("Testing %s...\n", p.c_str());

    REQUIRE(element->hasProperty("radius")    == element->hasProperty("diameter"));
    
    if (element->hasProperty("radius")) {
      printf("  - Testing for diameter <=> radius equivalence...\n");
      for (auto i = 0; i < 100; ++i) {
        Real radius = 1e-2 * RZ_URANDSIGN + 1e-1;
        auto val = element->get("radius");
        REQUIRE(val.type() == RealValue);
        REQUIRE(element->set("radius", radius));
        val = element->get("diameter");
        REQUIRE(val.type() != UndefinedValue);
        REQUIRE(releq((Real) val, 2 * radius));
      }

      for (auto i = 0; i < 100; ++i) {
        Real diameter = 1e-2 * RZ_URANDSIGN + 1e-1;
        auto val = element->get("diameter");
        REQUIRE(val.type() == RealValue);
        REQUIRE(element->set("diameter", diameter));
        val = element->get("radius");
        REQUIRE(val.type() != UndefinedValue);
        REQUIRE(releq(val, .5 * diameter));
      }
    }

    if (element->hasProperty("focalLength") && element->hasProperty("curvature")) {
      bool isLens = p.find("Lens") != -1;
      Real mu;

      // For lenses, the relationship between curvature radius and focal
      // length is different, as it involves the reffractive index.
      if (isLens) {
        auto val = element->get("n");

        REQUIRE(val.type() != UndefinedValue);
        REQUIRE(val.type() == RealValue);

        mu = (Real) val;
      }

      printf("  - Testing for focal length <=> curvature radius equivalence...\n");
      for (auto i = 0; i < 100; ++i) {
        Real focalLength = 1e-2 * RZ_URANDSIGN + 1e-1;
        auto val = element->get("focalLength");
        REQUIRE(val.type() == RealValue);
        REQUIRE(element->set("focalLength", focalLength));
        val = element->get("curvature");
        REQUIRE(val.type() != UndefinedValue);

        if (isLens)
          REQUIRE(releq(val, 2 * focalLength * (mu - 1)));
        else
          REQUIRE(releq(val, 2 * focalLength));
      }

      for (auto i = 0; i < 100; ++i) {
        Real curvature = 1e-2 * RZ_URANDSIGN + 1e-1;
        auto val = element->get("curvature");
        REQUIRE(val.type() == RealValue);
        REQUIRE(element->set("curvature", curvature));
        val = element->get("focalLength");
        REQUIRE(val.type() != UndefinedValue);

        if (isLens)
          REQUIRE(releq(val, .5 * curvature / (mu - 1)));
        else
          REQUIRE(releq(val, .5 * curvature));
      }
    }

    delete element;
  }
}

TEST_CASE("Element port access", THIS_TEST_TAG)
{
  WorldFrame world("world");
  
  for (auto &p : Singleton::instance()->elementFactories()) {
    auto factory = Singleton::instance()->lookupElementFactory(p);
    REQUIRE(factory != nullptr);

    auto element = factory->make(p, &world, nullptr);
    REQUIRE(element != nullptr);

    printf("Checking ports of %s:\n", p.c_str());

    for (auto &port : element->ports()) {
      printf("  - %10s ", port.c_str());

      auto portObj = element->getPortFrame(port);
      REQUIRE(portObj != nullptr);
      REQUIRE(portObj->typeString() != nullptr);
      printf("[%s]\n", portObj->typeString());
    }

    delete element;
  }
}
