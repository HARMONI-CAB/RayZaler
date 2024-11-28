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

      Real asReal, newReal;
      std::string asString, newString;
      int64_t asInteger, newInteger;
      bool asBool, newBool;

      switch (val.type()) {
        case RealValue:
          asReal = std::get<Real>(val);
          if (isZero(asReal))
            asReal += 1e-1;
          else
            asReal *= 0.9;

          REQUIRE(element->set(prop, asReal));
          newReal = element->get(prop);
          printf("(%g -> %g)\n", asReal, newReal);
          REQUIRE(releq(asReal, newReal));
          break;

        case IntegerValue:
          asInteger = std::get<int64_t>(val);
          ++asInteger;
          REQUIRE(element->set(prop, asInteger));
          newInteger = element->get(prop);
          printf("%d -> %d\n", asInteger, newInteger);
          REQUIRE(asInteger == newInteger);
          break;

        case BooleanValue:
          // The optical property is read-only and should not be changed
          asBool = std::get<bool>(val);
          if (prop == "optical")
            REQUIRE(asBool);
          
          asBool = !asBool;

          if (prop == "optical")
            REQUIRE(!element->set(prop, asBool));
          else
            REQUIRE(element->set(prop, asBool));

          newBool = std::get<bool>(element->get(prop));
          printf("%d -> %d\n", asBool, newBool);

          if (prop == "optical")
            REQUIRE(newBool);
          else
            REQUIRE(newBool == asBool);
          break;

        case StringValue:
          asString = std::get<std::string>(val);
          asString += "-suffix";
          REQUIRE(element->set(prop, asString));
          newString = std::get<std::string>(element->get(prop));
          printf("(\"%s\" -> \"%s\")\n", asString.c_str(), newString.c_str());
          REQUIRE(newString == asString);
          break;
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
  }
}
