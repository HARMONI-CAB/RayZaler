#ifndef _RAYTRACING_ENGINE_H
#define _RAYTRACING_ENGINE_H

#include <stdint.h>
#include <Vector.h>
#include <vector>
#include <list>
#include <sys/time.h>

#define RZ_SPEED_OF_LIGHT 299792458 // m/s

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
    Complex *amplitude  = nullptr;
    Real *lengths       = nullptr;
    Real *cumOptLengths = nullptr;
    Real *normals       = nullptr; // Surface normals of the depart surface
    Real n              = 1.;
    uint64_t *mask      = nullptr;
    uint64_t *prevMask  = nullptr;

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
    hadRay(uint64_t index) const
    {
      return (~prevMask[index >> 6] & (1ull << (index & 63))) >> (index & 63);
    }

    inline bool
    extractRay(Ray &dest, uint64_t index, bool keepPruned = false) const
    {
      bool haveIt = keepPruned ? hadRay(index) : hasRay(index);
      
      if (!haveIt)
        return false;

      dest.origin.setFromArray(origins + 3 * index);
      Vec3 diff = Vec3(destinations + 3 * index) - dest.origin;
      dest.length = diff.norm();

      if (isZero(dest.length))
        return false;

      dest.direction = diff / dest.length;

      // dest.direction.setFromArray(directions + 3 * index);
      
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
      Real m_K = 2 * M_PI;      // Wavenumber
      RayBeam *m_beam = nullptr;
      bool m_beamDirty = true;
      bool m_notificationPendig = false;
      const ReferenceFrame *m_current = nullptr;

      void toBeam();  // From m_rays to beam->origins and beam->directions
      void toRays(bool keepPruned = false);  // From beam->destinations and beam->directions to rays

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

      inline void
      setK(Real K)
      {
        m_K = K;
      }

      inline Real
      K() const
      {
        return m_K;
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

      // Update normals of the current beams
      void updateNormals();
      
      // Refresh ray origins
      void updateOrigins();

      // Adjust the current beam so that the rays coming to this surface look
      // like coming from infinity. This implies that:
      //
      // - We need to cast rays to the intercept surface
      // - We compute the intercept points by means of intercept()
      // - We update the beam the following way:
      //   1. We adjust the destination point
      //   2. We keep the direction here (encodes the arrival direction)
      //   3. We set the m_currentNormal to the intercept normal

      void traceFromInfinity(
        const ReferenceFrame *surface,
        const RayTransferProcessor *processor);

      // Deliver to an existing point of the next surface
      void traceRandom(
        const ReferenceFrame *surface,
        const RayTransferProcessor *next);

      // Compute Kirchhoff integral for all rays.
      void integrateKirchhoff();

      // Integrate phase for regular Snell
      void propagatePhase();

      // Update directions
      void updateDirections();

      // Clear m_ray, process the beam and extract unpruned rays
      void transfer(const RayTransferProcessor *);

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

