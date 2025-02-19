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


#ifndef _RAY_TRACING_HEURISTICS_DUMMY_H
#define _RAY_TRACING_HEURISTICS_DUMMY_H

#include <RayTracingHeuristic.h>

namespace RZ {
  class DummyHeuristic : public RayTracingHeuristic {
    public:
      DummyHeuristic(
        RayTracingHeuristicFactory *,
        OMModel *model);

      virtual void updateVisibility(const RayBeam &) override;
      virtual ~DummyHeuristic() override;

  };

  class DummyHeuristicFactory : public RayTracingHeuristicFactory {
    public:
      virtual std::string name() const override;
      virtual RayTracingHeuristic *make(OMModel *model) override;
  };
}

#endif // _RAY_TRACING_HEURISTICS_DUMMY_H
