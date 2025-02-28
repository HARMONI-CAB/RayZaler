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
#include <RayTracingHeuristic.h>
#include <OMModel.h>
#include <CPURayTracingEngine.h>
#include <Singleton.h>

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
  size_t n = 0;

  for (auto constSurf : path->m_sequence) {
    OpticalSurface *surface = const_cast<OpticalSurface *>(constSurf);

    m_engine->setCurrentStage(surface->name, n, path->m_sequence.size());

    m_engine->castTo(surface);

    if (props.listener != nullptr && props.listener->cancelled())
      return false;

    m_engine->beam()->computeInterceptStatistics(surface);

    // Save intermediate rays for representation
    if (props.beamElement != nullptr)
      m_engine->beam()->extractRays(
        m_intermediateRays,
          OriginPOV | BeamIsSurfaceRelative | ExtractIntercepted,
        surface);

    m_engine->transmitThrough(surface);

    m_engine->updateOrigins(); // Destinations == origins

    if (m_engine->cancelled())
      return false;

    ++n;
  }

  if (props.beamElement != nullptr)
    m_engine->beam()->extractRays(
      m_intermediateRays,
      OriginPOV | ExtractVignetted);

  return true;
}

void
Simulation::initNSBeam()
{
  if (m_NSBeam != nullptr) {
    delete m_NSBeam;
    m_NSBeam = nullptr;
  }

  m_transferredRays = 0;
  m_NSBeam = new RayBeam(m_engine->beam()->count, true);
  m_NSBeam->pruneAll();
}

bool
Simulation::traceNonSequential(TracingProperties const &props)
{
  unsigned int propagations = 0;

  auto factory =
    Singleton::instance()->lookupRayTracingHeuristicFactory(props.heuristic);

  if (factory == nullptr)
    throw std::runtime_error("Unrecognized heuristic type `" + props.heuristic + "'");

  if (m_heuristic != nullptr) {
    delete m_heuristic;
    m_heuristic = nullptr;
  }

  m_heuristic = factory->make(m_model);

  /* Algorithm is as follows:
     1. Loop
        1. Obtain the visible boundaries from the heuristic
        2. For each boundary, intersect beam with it and keep result.
        3. Traverse all boundaries and create new beams with the 
           non-vignetted rays and smallest positive advance
        4. Transmit all beams and merge them altogether.
        5. Save intermediate beam
     2. Repeat from 1 up to maxPropagations or no rays left, calc heuristic

     Perhaps we can optimize things the following way:
        1. Introduce a new column in the beam: the MediumBoundary
        2. Store a all-pruned beam and add a method to determine
           which rays are upgraded and which ones are not. We can start
           by copying the relevant fields of the beam.
    */

  auto tempBeam = m_engine->makeBeam();

  do {
    m_transferredRays = 0;
    
    // Non sequential beams are all-pruned by default
    auto nsBeam   = m_engine->makeBeam(true);
    
    //
    // In the engine: 
    //   - Make non-sequential beam.
    //   - Remove the concept of current surface. Just specify castTo(Surface)
    //   - 
    m_heuristic->updateVisibility(*m_engine->beam());
    auto candidates = m_heuristic->visibleList();

    printf("Propagation: main beam %p, NS beam %p\n", m_engine->beam(), nsBeam);
    
    // Build the non-sequential beam
    size_t n = 0;
    for (auto surface : candidates) {
      assert(surface != nullptr);

      m_engine->setCurrentStage(surface->name, n, candidates.size());

      // Convert this beam to relative and store it in tempBeam
      m_engine->beam()->toRelative(tempBeam, surface->frame);

      // Cast all these rays to the current surface
      m_engine->castTo(surface, tempBeam);

      // Update this non-sequential beam from the temporary beam
      m_transferredRays += nsBeam->updateFromVisible(surface, tempBeam);

      if (m_engine->cancelled())
        return false;

      ++n;
    }

    // The non sequential beam is ready, pass to ray tracer
    m_engine->setMainBeam(nsBeam);

    // Compute statistics on intercepted rays
    m_engine->beam()->computeInterceptStatistics();

    // Save intermediate rays for representation
    if (props.beamElement != nullptr)
      m_engine->beam()->extractRays(
        m_intermediateRays,
          OriginPOV 
        | BeamIsSurfaceRelative
        | ExtractIntercepted);
    
    printf("Total: %d\n\n", m_transferredRays);
    
    // Transmit through all these surfaces
    m_engine->transmitThroughIntercepted();

    m_engine->updateOrigins();

    if (m_engine->cancelled())
      return false;

  } while (++propagations < props.maxPropagations && m_transferredRays > 0);

  // Include straylight rays
  if (props.beamElement != nullptr)
    m_engine->beam()->extractRays(
      m_intermediateRays,
      OriginPOV | ExtractVignetted);
  
  return true;
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
