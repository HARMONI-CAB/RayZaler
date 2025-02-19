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


#include <RayTracingHeuristics/Dummy.h>
#include <OMModel.h>

using namespace RZ;

DummyHeuristic::DummyHeuristic(
  RayTracingHeuristicFactory *factory,
  OMModel *model) : RayTracingHeuristic(factory, model)
{
  auto const &all = model->allOpticalElements();

  visibleList().assign(all.begin(), all.end());
}

void
DummyHeuristic::updateVisibility(const RayBeam &)
{
  // Do nothing.
}

DummyHeuristic::~DummyHeuristic()
{

}


std::string
DummyHeuristicFactory::name() const
{
  return "DummyHeuristic";
}


RayTracingHeuristic *
DummyHeuristicFactory::make(OMModel *model)
{
  return new DummyHeuristic(this, model);
}
