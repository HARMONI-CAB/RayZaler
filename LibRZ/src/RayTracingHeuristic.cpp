//
//  Copyright (c) 2025 Gonzalo José Carracedo Carballal
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


#include <RayTracingHeuristic.h>

using namespace RZ;

OMModel *
RayTracingHeuristic::model() const
{
  return m_model;
}

std::list<OpticalElement *> &
RayTracingHeuristic::visibleList()
{
  return m_visible;
}

std::list<OpticalElement *> const &
RayTracingHeuristic::visibleList() const
{
  return m_visible;
}

RayTracingHeuristic::RayTracingHeuristic(
  RayTracingHeuristicFactory *factory,
  OMModel *model)
{
  m_model   = model;
  m_factory = factory;
}

RayTracingHeuristicFactory *
RayTracingHeuristic::factory() const
{
  return m_factory;
}

RayTracingHeuristic::~RayTracingHeuristic()
{

}
