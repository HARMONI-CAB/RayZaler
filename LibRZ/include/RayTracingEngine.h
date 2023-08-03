#ifndef _RAYTRACING_ENGINE_H
#define _RAYTRACING_ENGINE_H

#include <stdint.h>
#include <Vector.h>
#include <list>

namespace RZ {
  class ReferenceFrame;
  
  struct Ray {
    // Defined by input
    Vec3 origin;
    Vec3 direction;

    // Incremented by tracer
    Real length = 0;
  };

  struct RayBeam {
    uint64_t count      = 0;
    uint64_t allocation = 0;
    Real *origins       = nullptr;
    Real *directions    = nullptr;
    Real *destinations  = nullptr;
    Real *lengths       = nullptr;
    uint64_t *mask      = nullptr;

    inline void
    prune(uint64_t c)
    {
      mask[c >> 6] |= 1ull << (c & 63);
    }

    inline bool
    hasRay(uint64_t index)
    {
      return (~mask[index >> 6] & (1ull << (index & 63))) >> (index & 63);
    }

    inline bool
    extractRay(Ray &dest, uint64_t index)
    {
      if (!hasRay(index))
        return false;

      dest.origin.x = origins[3 * index + 0];
      dest.origin.y = origins[3 * index + 1];
      dest.origin.z = origins[3 * index + 2];

      dest.direction.x = directions[3 * index + 0];
      dest.direction.y = directions[3 * index + 1];
      dest.direction.z = directions[3 * index + 2];

      dest.length = lengths[index];

      return true;
    }

    virtual void allocate(uint64_t);
    virtual void deallocate();
    
    void clearMask();
    RayBeam(uint64_t);
    ~RayBeam();
  };

  class RayTransferProcessor {
  public:
    virtual std::string name() const = 0;
    virtual void process(RayBeam &, const ReferenceFrame *) const = 0;
  };

  class RayTracingEngine {
      std::list<Ray> m_rays;
      bool m_raysDirty = false;

      RayBeam *m_beam = nullptr;
      bool m_beamDirty = true;
      
      const ReferenceFrame *m_current = nullptr;

      void toBeam();  // From m_rays to beam->origins and beam->directions
      void toRays();  // From beam->destinations and beam->directions to rays

    protected:
      virtual RayBeam *makeBeam();
      virtual void cast(Point3 const &center,  Vec3 const &normal) = 0;
      
    public:
      inline RayBeam *
      beam() const
      {
        return m_beam;
      }


      RayTracingEngine();
      virtual ~RayTracingEngine();

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
  };
}

#endif // _RAYTRACING_ENGINE_H

