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

#ifndef _SIMULATION_H
#define _SIMULATION_H

#include <string>
#include <list>
#include <RayTracingEngine.h>

namespace RZ {
  class RayBeamElement;
  class RayTracingHeuristic;
  class OMModel;

  enum TracingType {
    Sequential,
    NonSequential
  };

  struct TracingProperties {
    TracingType     type                  = Sequential;
    RayBeamElement *beamElement           = nullptr;
    bool            clearPrevious         = true;
    bool            clearDetectors        = true;
    std::string     path;
    const std::list<Ray> *pRays           = nullptr;
    std::list<Ray>        rays;
    std::string           heuristic       = "dummy";
    unsigned int          maxPropagations = 1000;
    const struct timeval *startTime       = nullptr;
    RayTracingProcessListener *listener   = nullptr;
  };

  class Simulation {
      OMModel          *m_model  = nullptr;
      RayTracingEngine *m_engine = nullptr;
      RayBeam          *m_NSBeam = nullptr;
      uint64_t          m_transferredRays = 0;
      std::list<Ray>    m_intermediateRays;
      RayTracingHeuristic *m_heuristic = nullptr;
      struct timeval    m_lastTick;

      bool traceSequential(TracingProperties const &);
      bool traceNonSequential(TracingProperties const &);
      void initNSBeam();

    public:
      inline RayTracingEngine *
      engine() const
      {
        return m_engine;
      }
      
      Simulation(OMModel *model, std::string const &engine = "cpu");
      ~Simulation();

      bool trace(TracingProperties const &);
      struct timeval lastTick() const;
  };
}

#endif // _SIMULATION_H
