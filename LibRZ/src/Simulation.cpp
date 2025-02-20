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

#include <Simulation.h>
#include <OpticalElement.h>
#include <Elements/RayBeamElement.h>
#include <OMModel.h>
#include <CPURayTracingEngine.h>

using namespace RZ;

Simulation::Simulation(OMModel *model, std::string const &engine)
{
  if (engine != "cpu")
    throw std::runtime_error("Unrecognized simulation engine `" + engine + "'");

  m_model = model;
  m_engine = new CPURayTracingEngine;
}

Simulation::~Simulation()
{
  if (m_engine != nullptr)
    delete m_engine;
}

bool
Simulation::traceSequential(TracingProperties const &props)
{
  const OpticalPath *path = m_model->lookupOpticalPathOrEx(props.path);

  for (auto p : path->m_sequence) {
    m_engine->setCurrentSurface(p);
    m_engine->trace();

    if (props.listener != nullptr && props.listener->cancelled())
      return false;

    m_engine->transfer();

    if (props.beamElement != nullptr) {
      auto rays = m_engine->getRays(true);

      m_intermediateRays.insert(
        m_intermediateRays.end(),
        rays.begin(),
        rays.end());
    }
    
    m_engine->updateOrigins(); // Destinations == origins

    if (m_engine->cancelled())
      return false;
  }

  return true;
}

bool
Simulation::traceNonSequential(TracingProperties const &props)
{
  if (props.heuristic != "dummy")
    throw std::runtime_error("Unsupported heuristic");
  
  return false;
}

bool
Simulation::trace(TracingProperties const &props)
{
  bool ok = false;

  const std::list<Ray> *pRays = props.pRays != nullptr 
    ? props.pRays 
    : &props.rays;
  
  if (props.clearPrevious)
    m_intermediateRays.clear();

  m_engine->setListener(props.listener);
  m_engine->clear(); // Reset previous simulation

  // Clear all detectors, if requested
  if (props.clearDetectors)
    for (auto p : m_model->detectors())
      m_model->lookupDetectorOrEx(p)->clear();

  m_engine->pushRays(*pRays);

  if (props.startTime != nullptr)
    m_engine->setStartTime(*props.startTime);
  else
    m_engine->tick();

  switch (props.type) {
    case Sequential:
      ok = traceSequential(props);
      break;

    case NonSequential:
      ok = traceNonSequential(props);
      break;

    default:
      throw std::runtime_error("Unrecognized simulation type");
  }

  if (ok && props.beamElement != nullptr)
    props.beamElement->setList(m_intermediateRays);

  m_lastTick = m_engine->lastTick();

  return ok;
}

struct timeval
Simulation::lastTick() const
{
  return m_lastTick;
}
