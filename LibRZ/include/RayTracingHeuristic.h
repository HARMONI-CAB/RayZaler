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

#ifndef _RAY_TRACING_HEURISTIC_H
#define _RAY_TRACING_HEURISTIC_H

#include <list>
#include <string>

namespace RZ {
  class OMModel;
  class RayBeam;
  class OpticalElement;
  class RayTracingHeuristicFactory;

  class RayTracingHeuristic {
      OMModel                    *m_model = nullptr;
      std::list<OpticalElement *> m_visible;
      RayTracingHeuristicFactory *m_factory = nullptr;

    protected:
      OMModel *model() const;
      std::list<OpticalElement *> &visibleList();

    public:
      std::list<OpticalElement *> const &visibleList() const;

      RayTracingHeuristic(RayTracingHeuristicFactory *, OMModel *model);
      RayTracingHeuristicFactory *factory() const;
      virtual void updateVisibility(const RayBeam &) = 0;
      virtual ~RayTracingHeuristic();
  };

  class RayTracingHeuristicFactory {
    public:
      virtual std::string name() const = 0;
      virtual RayTracingHeuristic *make(OMModel *model) = 0;
  };
}

#endif // _RAY_TRACING_HEURISTIC_H
