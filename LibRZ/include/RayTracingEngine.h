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

#include <stdint.h>
#include <Vector.h>
#include <vector>
#include <list>
#include <map>
#include <sys/time.h>
#include "Random.h"

#define RZ_SPEED_OF_LIGHT 299792458 // m/s
#define RZ_WAVELENGTH     555e-9

namespace RZ {
  class ReferenceFrame;
  class OpticalSurface;

  struct Ray {
    // Defined by input
    Vec3 origin;
    Vec3 direction;

    // Incremented by tracer
    Real length = 0;
    Real cumOptLength = 0;

    // Defines whether the ray is susceptible to vignetting
    bool chief = false;

    RZ::Real wavelength = RZ_WAVELENGTH;

    // Defined by the user
    uint32_t id = 0;
  };

  class RayList : public std::list<RZ::Ray, std::allocator<RZ::Ray>> { };

  struct RayBeamStatistics {
    uint64_t intercepted = 0;
    uint64_t pruned      = 0;

    inline RayBeamStatistics &
    operator +=(RayBeamStatistics const &existing)
    {
      intercepted += existing.intercepted;
      pruned      += existing.pruned;
      return *this;
    }
  };

  struct RayBeam {
    uint64_t count       = 0;
    uint64_t allocation  = 0;
    
    std::map<uint32_t, RayBeamStatistics> statistics;

    Real *origins       = nullptr;
    Real *directions    = nullptr;
    Real *destinations  = nullptr;
    Complex *amplitude  = nullptr;
    Real *lengths       = nullptr;
    Real *cumOptLengths = nullptr;
    Real *normals       = nullptr; // Surface normals of the depart surface
    Real *wavelengths   = nullptr;
    uint32_t *ids       = nullptr;

    Real n              = 1.;
    uint64_t *mask      = nullptr;
    uint64_t *prevMask  = nullptr;
    uint64_t *chiefMask = nullptr;

    const OpticalSurface *hitSaveSurface = nullptr;
    
    inline void
    setRefractiveIndex(Real newN)
    {
      n = newN;
    }

    inline void
    setVacuum()
    {
      setRefractiveIndex(1.);
    }

    inline bool
    isChief(uint64_t index) const
    {
      return (chiefMask[index >> 6] & (1ull << (index & 63))) >> (index & 63);
    }

    inline bool
    hasRay(uint64_t index) const
    {
      return (~mask[index >> 6] & (1ull << (index & 63))) >> (index & 63);
    }

    inline void
    prune(uint64_t c)
    {
      if (!isChief(c) && hasRay(c))
        mask[c >> 6] |= 1ull << (c & 63);
    }

    inline bool
    setChiefRay(uint64_t c)
    {
      if (!hasRay(c))
        return false;

      chiefMask[c >> 6] |= 1ull << (c & 63);
      return true;
    }

    inline bool
    unsetsetChiefRay(uint64_t c)
    {
      if (!hasRay(c))
        return false;

      chiefMask[c >> 6] &= ~(1ull << (c & 63));
      return true;
    }

    inline bool
    hadRay(uint64_t index) const
    {
      return (~prevMask[index >> 6] & (1ull << (index & 63))) >> (index & 63);
    }

    inline bool
    extractRay(Ray &dest, uint64_t index, bool keepPruned = false) const
    {
      bool haveIt = keepPruned ? hadRay(index) : hasRay(index);
      
      if (!haveIt || lengths[index] < 0)
        return false;

      dest.origin.setFromArray(origins + 3 * index);
      Vec3 diff = Vec3(destinations + 3 * index) - dest.origin;
      dest.length = diff.norm();
      dest.chief  = isChief(index);
      dest.wavelength = wavelengths[index];

      if (isZero(dest.length))
        return false;

      dest.direction = diff / dest.length;
      dest.id = ids[index];
      // dest.direction.setFromArray(directions + 3 * index);
      
      dest.cumOptLength = cumOptLengths[index];

      return true;
    }

    inline bool
    extractPartialRay(Ray &dest, uint64_t index, bool keepPruned = false) const
    {
      bool haveIt = keepPruned ? hadRay(index) : hasRay(index);
      
      if (!haveIt)
        return false;

      dest.id = ids[index];
      dest.origin.setFromArray(destinations + 3 * index);
      dest.direction.setFromArray(directions + 3 * index);
      dest.wavelength = wavelengths[index];

      if (lengths[index] < 0)
        dest.direction = -dest.direction;
      
      dest.chief = isChief(index);
      dest.cumOptLength = cumOptLengths[index];

      return true;
    }

    virtual void interceptDone(uint64_t index);
    virtual void allocate(uint64_t);
    virtual void deallocate();
    
    void clearMask();
    void computeStatistics();
    void clearStatistics();

    RayBeam(uint64_t);
    ~RayBeam();
  };

  class SurfaceShape;

  class RayTransferProcessor {
    SurfaceShape   *m_surfaceShape = nullptr;
    ExprRandomState m_randState;
    bool            m_reversible = false;

  protected:
    inline void
    setSurfaceShape(SurfaceShape *ap)
    {
      m_surfaceShape = ap;
    }

    inline void
    setReversible(bool rev)
    {
      m_reversible = rev;
    }

  public:
    inline bool
    reversible() const
    {
      return m_reversible;
    }
    
    inline SurfaceShape *
    surfaceShape() const
    {
      return m_surfaceShape;
    }

    template <class T>
    inline T *
    surfaceShape()
    {
      return static_cast<T *>(surfaceShape());
    }
    
    template <class T>
    inline T const *
    surfaceShape() const
    {
      return static_cast<const T *>(surfaceShape());
    }

    static inline void
    snell(Vec3 &u, Vec3 const &normal, Real muIORatio)
    {
      Vec3 nXu = muIORatio * normal.cross(u);
      u        = -normal.cross(nXu) - normal * sqrt(1 - nXu * nXu);
    }

    static inline Vec3
    snell(Vec3 const &u, Vec3 const &normal, Real muIORatio)
    {
      Vec3 nXu = muIORatio * normal.cross(u);

      return -normal.cross(nXu) - normal * sqrt(1 - nXu * nXu);
    }

    static inline void
    reflection(Vec3 &u, Vec3 const &normal)
    {
      u -= 2 * (u * normal) * normal;
    }

    static inline Vec3
    reflection(Vec3 const &u, Vec3 const &normal)
    {
      return u - 2 * (u * normal) * normal;
    }

    inline ExprRandomState const &
    constRandState() const
    {
      return m_randState;
    }

    inline ExprRandomState &
    randState() const
    {
      return const_cast<ExprRandomState &>(m_randState);
    }

    virtual std::string name() const = 0;
    virtual void process(RayBeam &, const ReferenceFrame *) const = 0;
    virtual ~RayTransferProcessor();
  };

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
      std::list<Ray> m_rays;
      bool m_raysDirty = false;
      std::vector<Real> m_currNormals;
      Real m_dA = 1.;     // Area element of the departure surface
      Real m_currdA = 1.; // Area element of the current surface
      RayBeam *m_beam = nullptr;
      bool m_beamDirty = true;
      bool m_notificationPendig = false;
      const ReferenceFrame *m_currentFrame = nullptr;
      const OpticalSurface *m_currentSurface = nullptr;
      unsigned int m_numStages = 0;
      unsigned int m_currStage = 0;

      void toBeam();  // From m_rays to beam->origins and beam->directions
      void toRays(bool keepPruned = false);  // From beam->destinations and beam->directions to rays

      RayTracingProcessListener *m_listener = nullptr; // Always borrowed

      struct timeval m_start;

    protected:
      virtual RayBeam *makeBeam();
      virtual void cast(Point3 const &center,  Vec3 const &normal, bool reversible) = 0;
      void rayProgress(uint64_t num, uint64_t total);

    public:
      inline RayBeam *
      beam() const
      {
        return m_beam;
      }

      RayTracingEngine();
      virtual ~RayTracingEngine();

      // This sets the event listener, to receive periodic updates
      RayTracingProcessListener *listener() const;
      void setListener(RayTracingProcessListener *);
      
      // This clears all the rays
      void clear();

      // This adds a new ray to the list
      void pushRay(
        Point3 const &origin,
        Vec3 const &direction,
        Real length = 0,
        uint32_t id = 0);
      void pushRays(std::list<Ray> const &);
      
      // Set the current surface. This also notifies the state.
      void setCurrentSurface(
        const OpticalSurface *surf,
        unsigned i = 0,
        unsigned total = 0);

      // Intersect with this surface. It needs to check if the beam is up to
      // date, and recreated it with toBeam() if necessary.
      // Then, it will call to cast() and compute beam->destinations and lengths
      void trace();

      // Update normals of the current beams
      void updateNormals();
      
      // Refresh ray origins
      void updateOrigins();

      // Integrate phase for regular Snell
      void propagatePhase();

      // Clear m_ray, process the beam and extract unpruned rays
      void transfer();

      // Clear m_ray, process the beam, set random targets 
      // Return the output rays, after transfer
      std::list<Ray> const &getRays(bool keepPruned = false);

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

