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
#define THIS_TEST_TAG "[TopLevel]"

#include <TopLevelModel.h>
#include <catch2/catch_test_macros.hpp>

using namespace RZ;

TEST_CASE("Empty model", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString("");
  REQUIRE(model != nullptr);

  delete model;
}

TEST_CASE("Basic element creation", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString("BlockElement block;");
  REQUIRE(model != nullptr);

  auto element = model->lookupElement("block");
  REQUIRE(element != nullptr);

  delete model;
}

TEST_CASE("Element property resolution", THIS_TEST_TAG)
{
  bool exceptOnExisting = false;
  try {
    auto model = TopLevelModel::fromString("BlockElement block (width = 1);");
    delete model;
  } catch (std::runtime_error const &e) {
    exceptOnExisting = true;
  }

  REQUIRE(!exceptOnExisting);

  bool exceptOnNonExisting = false;
  try {
    auto model = TopLevelModel::fromString("BlockElement block (nonExistent = 1);");
    delete model;
  } catch (std::runtime_error const &e) {
    exceptOnNonExisting = true;
  }

  REQUIRE(exceptOnNonExisting);

  bool exceptOnString = false;
  try {
    auto model = TopLevelModel::fromString("BlockElement block (width = \"1\");");
    delete model;
  } catch (std::runtime_error const &e) {
    exceptOnString = true;
  }

  REQUIRE(exceptOnString);

  bool exceptOnReal = false;
  try {
    auto model = TopLevelModel::fromString("StlMesh mesh(file = 3);");
    delete model;
  } catch (std::runtime_error const &e) {
    exceptOnReal = true;
  }

  REQUIRE(exceptOnReal);
}

TEST_CASE("DOF + translation", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(
    "dof x = 0;"
    "dof y = 0;"
    "dof z = 0;"
    "translate(dx = x, dy = y, dz = z) BlockElement block;"
  );

  REQUIRE(model != nullptr);

  auto element = model->lookupElement("block");
  REQUIRE(element != nullptr);

  auto frame = element->parentFrame();
  REQUIRE(frame != nullptr);

  for (auto i = 0; i < 100; ++i) {
    Real x = RZ_URANDSIGN;
    Real y = RZ_URANDSIGN;
    Real z = RZ_URANDSIGN;

    model->setDof("x", x);
    model->setDof("y", y);
    model->setDof("z", z);

    REQUIRE(releq(x, frame->getCenter().x));
    REQUIRE(releq(y, frame->getCenter().y));
    REQUIRE(releq(z, frame->getCenter().z));
  }

  delete model;
}

TEST_CASE("Variables + translation", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(
    "dof u = 0;"
    "dof v = 0;"
    "var x = u + v;"
    "var y = u - v;"
    "translate(dx = x, dy = y) BlockElement block;"
  );

  REQUIRE(model != nullptr);

  auto element = model->lookupElement("block");
  REQUIRE(element != nullptr);

  auto frame = element->parentFrame();
  REQUIRE(frame != nullptr);

  for (auto i = 0; i < 100; ++i) {
    Real u = RZ_URANDSIGN;
    Real v = RZ_URANDSIGN;
    Real x = u + v;
    Real y = u - v;

    model->setDof("u", u);
    model->setDof("v", v);

    REQUIRE(releq(x, frame->getCenter().x));
    REQUIRE(releq(y, frame->getCenter().y));
  }

  delete model;
}

TEST_CASE("Element rotation", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(
    "dof ux    = 0;"
    "dof uy    = 0;"
    "dof uz    = 0;"
    "dof alpha = 0;"
    "rotate(alpha, ux, uy, uz) BlockElement block;"
  );

  REQUIRE(model != nullptr);

  auto element = model->lookupElement("block");
  REQUIRE(element != nullptr);

  auto frame = element->parentFrame();
  REQUIRE(frame != nullptr);

  // Orientation should be a matrix whose columns are the new axes.
  printf("Rotate around X...\n");
  model->setDof("ux", 1);
  model->setDof("uy", 0);
  model->setDof("uz", 0);

  for (auto i = 0; i < 100; ++i) {
    Real angle = RZ_URANDSIGN * M_PI;
    Vec3 vy    = Vec3(0,  cos(angle), sin(angle));
    Vec3 vz    = Vec3(0, -sin(angle), cos(angle));

    model->setDof("alpha", rad2deg(angle));

    REQUIRE(frame->getOrientation().t().vx() == Vec3::eX());
    REQUIRE(frame->getOrientation().t().vy() == vy);
    REQUIRE(frame->getOrientation().t().vz() == vz);
  }

  printf("Rotate around Y...\n");
  model->setDof("ux", 0);
  model->setDof("uy", 1);
  model->setDof("uz", 0);

  for (auto i = 0; i < 100; ++i) {
    Real angle = RZ_URANDSIGN * M_PI;
    Vec3 vz    = Vec3(sin(angle), 0,  cos(angle));
    Vec3 vx    = Vec3(cos(angle), 0, -sin(angle));

    model->setDof("alpha", rad2deg(angle));

    REQUIRE(frame->getOrientation().t().vx() == vx);
    REQUIRE(frame->getOrientation().t().vy() == Vec3::eY());
    REQUIRE(frame->getOrientation().t().vz() == vz);
  }

  printf("Rotate around Z...\n");
  model->setDof("ux", 0);
  model->setDof("uy", 0);
  model->setDof("uz", 1);

  for (auto i = 0; i < 100; ++i) {
    Real angle = RZ_URANDSIGN * M_PI;
    Vec3 vx    = Vec3( cos(angle), sin(angle), 0);
    Vec3 vy    = Vec3(-sin(angle), cos(angle), 0);

    model->setDof("alpha", rad2deg(angle));

    REQUIRE(frame->getOrientation().t().vx() == vx);
    REQUIRE(frame->getOrientation().t().vy() == vy);
    REQUIRE(frame->getOrientation().t().vz() == Vec3::eZ());
  }

  delete model;
}

TEST_CASE("Element rotation and translation", THIS_TEST_TAG)
{
  auto model = TopLevelModel::fromString(
    "dof ux    = 0;"
    "dof uy    = 0;"
    "dof uz    = 0;"
    "dof alpha = 0;"
    "rotate(alpha, ux, uy, uz) {"
    "  translate(dx = 1) BlockElement blockDx;"
    "  translate(dy = 1) BlockElement blockDy;"
    "  translate(dz = 1) BlockElement blockDz;"
    "}"
  );

  REQUIRE(model != nullptr);

  auto blockDx = model->lookupElement("blockDx");
  REQUIRE(blockDx != nullptr);
  auto frameDx = blockDx->parentFrame();
  REQUIRE(frameDx != nullptr);

  auto blockDy = model->lookupElement("blockDy");
  REQUIRE(blockDy != nullptr);
  auto frameDy = blockDy->parentFrame();
  REQUIRE(frameDy != nullptr);

  auto blockDz = model->lookupElement("blockDz");
  REQUIRE(blockDz != nullptr);
  auto frameDz = blockDz->parentFrame();
  REQUIRE(frameDz != nullptr);

  printf("Rotate around X...\n");
  model->setDof("ux", 1);
  model->setDof("uy", 0);
  model->setDof("uz", 0);

  for (auto i = 0; i < 100; ++i) {
    Real angle = RZ_URANDSIGN * M_PI;
    Vec3 dy    = Vec3(0,  cos(angle), sin(angle));
    Vec3 dz    = Vec3(0, -sin(angle), cos(angle));

    model->setDof("alpha", rad2deg(angle));

    REQUIRE(frameDx->getCenter() == Vec3::eX());
    REQUIRE(frameDy->getCenter() == dy);
    REQUIRE(frameDz->getCenter() == dz);
  }

  printf("Rotate around Y...\n");
  model->setDof("ux", 0);
  model->setDof("uy", 1);
  model->setDof("uz", 0);

  for (auto i = 0; i < 100; ++i) {
    Real angle = RZ_URANDSIGN * M_PI;
    Vec3 dz    = Vec3(sin(angle), 0,  cos(angle));
    Vec3 dx    = Vec3(cos(angle), 0, -sin(angle));

    model->setDof("alpha", rad2deg(angle));

    REQUIRE(frameDx->getCenter() == dx);
    REQUIRE(frameDy->getCenter() == Vec3::eY());
    REQUIRE(frameDz->getCenter() == dz);
  }

  printf("Rotate around Z...\n");
  model->setDof("ux", 0);
  model->setDof("uy", 0);
  model->setDof("uz", 1);

  for (auto i = 0; i < 100; ++i) {
    Real angle = RZ_URANDSIGN * M_PI;
    Vec3 dx    = Vec3( cos(angle), sin(angle), 0);
    Vec3 dy    = Vec3(-sin(angle), cos(angle), 0);

    model->setDof("alpha", rad2deg(angle));

    REQUIRE(frameDx->getCenter() == dx);
    REQUIRE(frameDy->getCenter() == dy);
    REQUIRE(frameDz->getCenter() == Vec3::eZ());
  }

  delete model;
}

