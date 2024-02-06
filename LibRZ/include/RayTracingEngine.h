#ifndef _RAYTRACING_ENGINE_H
#define _RAYTRACING_ENGINE_H

#include <stdint.h>
#include <Vector.h>
#include <list>
#include <sys/time.h>

namespace RZ {
  class ReferenceFrame;
  
  struct Ray {
    // Defined by input
    Vec3 origin;
    Vec3 direction;

    // Incremented by tracer
    Real length = 0;
    Real cumOptLength = 0;
  };

  class RayList : public std::list<RZ::Ray, std::allocator<RZ::Ray>> { };

  struct RayBeam {
    uint64_t count      = 0;
    uint64_t allocation = 0;
    Real *origins       = nullptr;
    Real *directions    = nullptr;
    Real *destinations  = nullptr;
    Real *lengths       = nullptr;
    Real *cumOptLengths = nullptr;
    Real n              = 1.;
    uint64_t *mask      = nullptr;

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

    inline void
    prune(uint64_t c)
    {
      mask[c >> 6] |= 1ull << (c & 63);
    }

    inline bool
    hasRay(uint64_t index) const
    {
      return (~mask[index >> 6] & (1ull << (index & 63))) >> (index & 63);
    }

    inline bool
    extractRay(Ray &dest, uint64_t index) const
    {
      if (!hasRay(index))
        return false;

      dest.origin.x = origins[3 * index + 0];
      dest.origin.y = origins[3 * index + 1];
      dest.origin.z = origins[3 * index + 2];

      dest.direction.x = directions[3 * index + 0];
      dest.direction.y = directions[3 * index + 1];
      dest.direction.z = directions[3 * index + 2];

      dest.length       = lengths[index];
      dest.cumOptLength = cumOptLengths[index];

      return true;
    }

    virtual void allocate(uint64_t);
    virtual void deallocate();
    
    void clearMask();
    RayBeam(uint64_t);
    ~RayBeam();
  };

  class GenericAperture;

  class RayTransferProcessor {
    GenericAperture *m_aperture = nullptr;

  protected:
    inline void defineAperture(GenericAperture *ap)
    {
      m_aperture = ap;
    }

  public:
    inline GenericAperture *aperture() const
    {
      return m_aperture;
    }

    template <class T>
    inline T *aperture()
    {
      return static_cast<T *>(aperture());
    }
    
    template <class T>
    inline T const *aperture() const
    {
      return static_cast<const T *>(aperture());
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

    virtual std::string name() const = 0;
    virtual void process(RayBeam &, const ReferenceFrame *) const = 0;
    virtual ~RayTransferProcessor();
  };

  enum RayTracingStageProgressType {
    PROGRESS_TYPE_TRACE,     // Tracing rays to capture surface
    PROGRESS_TYPE_TRANSFER,  // Transfering exit rays
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

      RayBeam *m_beam = nullptr;
      bool m_beamDirty = true;
      bool m_notificationPendig = false;
      const ReferenceFrame *m_current = nullptr;

      void toBeam();  // From m_rays to beam->origins and beam->directions
      void toRays();  // From beam->destinations and beam->directions to rays

      RayTracingProcessListener *m_listener = nullptr; // Always borrowed

      struct timeval m_start;

    protected:
      virtual RayBeam *makeBeam();
      virtual void cast(Point3 const &center,  Vec3 const &normal) = 0;
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
      void pushRay(Point3 const &origin, Vec3 const &direction, Real length = 0);
      void pushRays(std::list<Ray> const &);
      
      // Intersect with this surface. It needs to check if the beam is up to
      // date, and recreated it with toBeam() if necessary.
      // Then, it will call to cast() and compute beam->destinations and lengths
      void trace(const ReferenceFrame *surface);

      // Clear m_ray, process the beam and extract unpruned rays
      void transfer(const RayTransferProcessor *);

      // Return the output rays, after transfer
      std::list<Ray> const &getRays();

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

