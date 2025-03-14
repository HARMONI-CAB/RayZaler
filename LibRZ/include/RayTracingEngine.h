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

#ifndef _RAYTRACING_ENGINE_H
#define _RAYTRACING_ENGINE_H

#include <sys/time.h>
#include <cassert>

#include "RayBeam.h"

namespace RZ {
  class ReferenceFrame;
  class OpticalSurface;

  enum RayTracingStageProgressType {
    PROGRESS_TYPE_TRACE,     // Tracing rays to capture surface
    PROGRESS_TYPE_TRANSFER,  // Transfering exit rays
    PROGRESS_TYPE_KIRCHHOFF, // Integrating wavefronts
    PROGRESS_TYPE_CONFIG,    // Reconfigure model
  };

  class RayTracingProcessListener {
    public:
      virtual void stageProgress(
        RayTracingStageProgressType,
        std::string const &,
        unsigned int num,
        unsigned int total);
      virtual void rayProgress(uint64_t num, uint64_t total);

      virtual uint64_t rayNotifyInterval() const;
      virtual bool cancelled() const;
  };

  class RayTracingEngine {
      RayList  m_rays;
      bool     m_raysDirty = false;
      
      RayBeam *m_beam = nullptr;
      bool     m_beamDirty = true;
      bool     m_notificationPendig = false;

      std::string m_stageName;
      size_t      m_currStage = 0;
      size_t      m_numStages = 0;

      void toBeam();  // From m_rays to beam->origins and beam->directions
      void toRays(bool keepPruned = false);  // From beam->destinations and beam->directions to rays

      RayTracingProcessListener *m_listener = nullptr; // Always borrowed

      struct timeval m_start;

    protected:
      virtual void cast(const OpticalSurface *, RayBeam *) = 0;
      virtual void transmit(const OpticalSurface *, RayBeam *) = 0;

      void rayProgress(uint64_t num, uint64_t total);

    public:
      inline RayBeam *
      beam() const
      {
        return m_beam;
      }

      inline void
      setCurrentStage(std::string const &name, size_t current, size_t num)
      {
        m_stageName = name;
        m_currStage = current;
        m_numStages = num;
      }

      virtual RayBeam *makeBeam();
      virtual RayBeam *makeNSBeam();
      
      RayBeam *ensureMainBeam();

      RayTracingEngine();
      virtual ~RayTracingEngine();

      // This sets the event listener, to receive periodic updates
      RayTracingProcessListener *listener() const;
      void setListener(RayTracingProcessListener *);
      
      // This clears all the rays
      void clear();

      // This sets a beam that was initialized from somewhere else
      void setMainBeam(RayBeam *);

      // This adds a new ray to the list
      void pushRay(
        Point3 const &origin,
        Vec3 const &direction,
        Real length = 0,
        uint32_t id = 0);
      void pushRays(RayList const &);
      
      // Intersect with this surface. It needs to check if the beam is up to
      // date, and recreated it with toBeam() if necessary.
      // Then, it will call to cast() and compute beam->destinations and lengths
      void castTo(const OpticalSurface *, RayBeam *beam = nullptr);
      
      // Refresh ray origins
      void updateOrigins();

      // Clear m_ray, process the beam and extract unpruned rays
      void transmitThrough(const OpticalSurface *surface);
      void transmitThroughIntercepted(); // Equivalent to transmitThrough(nullptr)

      // Clear m_ray, process the beam, set random targets 
      // Return the output rays, after transfer
      RayList const &getRays(bool keepPruned = false);

      // Mark start of elapsed time counter
      void tick();

      // Get elapsed time
      void setStartTime(struct timeval const &tv);
      struct timeval lastTick() const;

      uint64_t tack() const;

      // Get if global progress notifications are pending
      bool notificationPending() const;
      void clearPendingNotifications();
      bool cancelled() const;

      void stageProgress(
          RayTracingStageProgressType,
          std::string const &,
          unsigned int num,
          unsigned int total);
        
  };
}

#endif // _RAYTRACING_ENGINE_H

