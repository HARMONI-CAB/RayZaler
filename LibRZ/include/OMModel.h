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

#ifndef _OMMODEL_H
#define _OMMODEL_H

#include <Vector.h>
#include <WorldFrame.h>
#include <ReferenceFrame.h>
#include <Element.h>
#include <Singleton.h>
#include <OpticalElement.h>
#include <Elements/Detector.h>
#include <Random.h>
#include <SkySampler.h>
#include <list>
#include <map>

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define RZ_DEFAULT_CCD_RESOLUTION 1024
#define RZ_DEFAULT_CCD_WIDTH      5e-2
#define RZ_WAVELENGTH             555e-9

namespace RZ {
  class OMModel;
  class GenericCompositeModel;
  class Simulation;
  
  //
  // An Opto-Mechanical model consists of:
  //
  // - A World reference frame
  // - Some other ancillary reference frames
  // - Elements, defined on top of other reference frames. Of which
  //   - Some may be detectors
  // - One or more optical paths
  // - A set of degrees of freedom (with their defaults)
  // - A set of parameters (with their defaults)
  // - A set of conversion rules
  //
  // Additionally, compound elements may be defined.
  //

  class ExpressionEvaluator;
  class RayBeamElement;
  class RayColoring;

  struct EvaluatedParameter {
    Real        value;
    std::string expression;
    ExpressionEvaluator *evaluator = nullptr;
    std::list<std::string> change_list;
  };


  struct ReferenceFrameContext {
    OMModel *m_model = nullptr;
    ReferenceFrame *m_last = nullptr;

    ReferenceFrameContext &rotate(Real, Vec3 const &);
    ReferenceFrameContext &rotate(Real, Real, Real, Real);

    ReferenceFrameContext &translate(Vec3 const &);
    ReferenceFrameContext &translate(Real, Real, Real);

    inline operator
    ReferenceFrame *() const
    {
      return m_last;
    }
  };

  enum BeamReference {
    SkyRelative,
    ElementRelative,
    PlaneRelative
  };

  enum BeamShape {
    Circular,
    Ring,
    Point,
    Custom
  };

  enum FNumReference {
    BeamDiameter,
    BeamLength,
  };

  struct BeamProperties {
    uint32_t id              = 0;
    BeamReference reference  = SkyRelative;
    BeamShape     shape      = Circular;
    bool          converging = true;

    union {
      const ReferenceFrame *frame = nullptr;
      const Element *element;
    };

    std::string path;
    Real length              = 10;           // [m]
    Real diameter            = .5;           // [m]
    Real wavelength          = 535e-9;
    unsigned int numRays     = 1000;
    bool random              = false;
    Vec3 direction           = -Vec3::eZ();  // [1]
    Vec3 offset              = Vec3::zero(); // [m]
    Real focusZ              = 0;            // [m]
    bool vignetting          = true;

    // Object structure
    SkyObjectShape objectShape = PointLike;
    Real angularDiameter     = M_PI / 6; // [1]
    std::string objectPath;

    // 
    // fNum = length / D This refers to beams that arrive as a circle.
    //
    inline void
    setFNum(Real fNum, FNumReference adjust = BeamDiameter)
    {
      if (adjust == BeamDiameter)
        diameter = length / fNum; 
      else
        length   = diameter * fNum;
    }

    //
    // For object-like sources sometimes it is interesting to describe the
    // illuminating cone froma f/#. The relationship between angular diameter
    // and f/# is given by:
    //
    // a = 2 atan (0.5 / f)
    //

    inline void
    setObjectFNum(Real fNum)
    {
      fNum = std::fabs(fNum);
      
      if (std::isinf(fNum)) {
        objectShape = PointLike;
      } else {
        objectShape = CircleLike;
        angularDiameter = 2 * std::atan(0.5 / fNum);
      }
    }

    inline void
    collimate()
    {
      focusZ = -std::numeric_limits<Real>::infinity();
    }

    inline void
    setElementRelative(const Element *element)
    {
      reference     = ElementRelative;
      this->element = element;
    }

    inline void
    setPlaneRelative(const ReferenceFrame *frame)
    {
      reference     = PlaneRelative;
      this->frame   = frame;
    }

    inline void
    setObjectShape(std::string const &shape)
    {
      if (shape == "point")
        this->objectShape = PointLike;
      else if (shape == "circular")
        this->objectShape = CircleLike;
      else if (shape == "extended")
        this->objectShape = Extended;
      else
        std::runtime_error("Unrecognized angular shape `" + shape + "'");
    }

    void debug() const;
  };

  class OMModel {
      std::list<ReferenceFrame *> m_frames;                  // Frame allocation
      std::map<std::string, ReferenceFrame *> m_nameToFrame; // Frame indexation

      std::list<Element *> m_elements;                  // Element allocation
      std::map<std::string, Element *> m_nameToElement; // Element indexation
      std::map<std::string, OpticalElement *> m_nameToOpticalElement; // Element indexation

      std::list<OpticalPath> m_paths;  // Optical path allocation
      std::map<std::string, OpticalPath *> m_nameToPath; // Path indexation
      std::map<std::string, Detector *> m_nameToDetector;

      // Convenience references
      ReferenceFrame *m_world = nullptr;
      
      // Convenience elements
      RayBeamElement *m_beam = nullptr;
      Simulation     *m_sim  = nullptr;

      bool registerElement(Element *);
      bool registerOpticalElement(OpticalElement *);
      bool registerDetector(Detector *);

      friend struct ReferenceFrameContext;

    protected:
      EvaluatedParameter *makeParameter(std::string const &, std::string const &);
      std::string genElementName(std::string const &type);
      std::string genReferenceFrameName(std::string const &type);

      bool registerDetectorAlias(std::string const &name, Detector *);

    public:
      OMModel();
      ~OMModel();

      bool registerFrame(ReferenceFrame *);
      void setFrameAlias(ReferenceFrame *, std::string const &name);
      Element *autoRegisterElement(Element *);

      inline ReferenceFrame *
      world() const
      {
        return m_world;
      }

      void linkWorld(ReferenceFrame *);
      
      template <class T>
      inline T *
      plug(
        std::string const &existing,
        std::string const &port,
        std::string const &type,
        std::string const &name = "")
      {
        std::string rwName = name;
        Element *element = lookupElementOrEx(existing);
        T *newElement;

        if (rwName.size() == 0)
          rwName = genElementName(type);

        if (lookupElement(rwName) != nullptr)
          return nullptr;

        return static_cast<T *>(
          autoRegisterElement(element->plug<T>(port, type, rwName)));
      }

      template <class T>
      inline T *
      plug(
        std::string const &existingFrame,
        std::string const &type,
        std::string const &name = "")
      {
        std::string rwName = name;
        ReferenceFrame *frame = lookupReferenceFrameOrEx(existingFrame);
        
        // Lookup factory for this element
        RZ::Singleton *sing = RZ::Singleton::instance();
        auto factory = sing->lookupElementFactory(type);

        if (factory == nullptr)
          throw std::runtime_error("Element class `" + type + "` does not exist");

        // Sanitize name
        if (rwName.size() == 0)
          rwName = genElementName(type);
        
        if (lookupElement(rwName) != nullptr)
          return nullptr;

        return static_cast<T *>(
          autoRegisterElement(factory->make(rwName, frame)));
      }
      
      void recalculate();
      RayBeamElement *beam() const;
      void clearBeam();
      void setBeamColoring(RayColoring const *coloring);

      ReferenceFrameContext rotate(Real, Vec3 const &, ReferenceFrame *parent = nullptr);
      ReferenceFrameContext rotate(Real, Real, Real, Real, ReferenceFrame *parent = nullptr);

      ReferenceFrameContext translate(Vec3 const &, ReferenceFrame *parent = nullptr);
      ReferenceFrameContext translate(Real, Real, Real, ReferenceFrame *parent = nullptr);

      // Optical path configuration
      bool addOpticalPath(std::string const &, std::list<std::string> const &); // Just elements
      bool addDetector(
        std::string const &name,
        ReferenceFrame *pFrame,
        unsigned int cols = RZ_DEFAULT_CCD_RESOLUTION,
        unsigned int rows = RZ_DEFAULT_CCD_RESOLUTION,
        Real width = RZ_DEFAULT_CCD_WIDTH,
        Real height = RZ_DEFAULT_CCD_WIDTH);

      bool addDetector(
        std::string const &name,
        std::string const &parentFrame,
        unsigned int cols = RZ_DEFAULT_CCD_RESOLUTION,
        unsigned int rows = RZ_DEFAULT_CCD_RESOLUTION,
        Real width = RZ_DEFAULT_CCD_WIDTH,
        Real height = RZ_DEFAULT_CCD_WIDTH);

      // Enumeration methods
      std::list<std::string> frames() const;
      std::list<std::string> elements() const;
      std::list<std::string> opticalElements() const;
      std::list<std::string> detectors() const;
      std::list<std::string> opticalPaths() const;
      std::list<std::string> elementHierarchy(std::string const &pfx = "") const;
      std::list<std::string> opticalElementHierarchy(std::string const &pfx = "") const;
      void boundingBox(Vec3 &, Vec3 &) const;

      // Convenience methods (fast enumeration)
      std::list<Element *> const &elementList() const;
      std::list<Element *> allElements() const;
      std::list<OpticalElement *> allOpticalElements() const;
      
      // Lookup methods
      ReferenceFrame *lookupReferenceFrame(std::string const &) const;
      Element *resolveElement(std::string const &) const;
      Element *lookupElement(std::string const &) const;
      OpticalElement *resolveOpticalElement(std::string const &) const;
      OpticalElement *lookupOpticalElement(std::string const &) const;
      Detector *lookupDetector(std::string const &) const;
      const OpticalPath *lookupOpticalPath(std::string const & = "") const;

      // Lookup methods (raise exception)
      ReferenceFrame *lookupReferenceFrameOrEx(std::string const &) const;
      Element *lookupElementOrEx(std::string const &) const;
      OpticalElement *lookupOpticalElementOrEx(std::string const &) const;
      Detector *lookupDetectorOrEx(std::string const &) const;
      const OpticalPath *lookupOpticalPathOrEx(std::string const & = "") const;

      // Raytracing methods
      bool trace(
        std::string const &path,
        std::list<RZ::Ray> const &rays,
        bool updateBeamElement = false,
        RayTracingProcessListener *listener = nullptr,
        bool clear = true,
        const struct timeval *startTime = nullptr,
        bool clearIntermediate = true);

      bool traceDiffraction(
        std::string const &path,
        std::list<RZ::Ray> const &rays,
        RayTracingProcessListener *listener = nullptr,
        bool clear = true,
        const struct timeval *startTime = nullptr);
      
      bool traceDefault(
        std::list<RZ::Ray> const &rays,
        bool updateBeamElement = false,
        RayTracingProcessListener *listener = nullptr,
        bool clear = true,
        const struct timeval *startTime = nullptr);

      struct timeval lastTracerTick() const;

      // Save images
      bool savePNG(
        std::string const &detector,
        std::string const &file);

      static void addSkyBeam(
        std::list<Ray> &dest,
        unsigned int number = 1000,
        Real radius = .5,
        Real azimuth = 0,
        Real elevation = 90,
        Real distance = 10,
        uint32_t id = 0,
        bool random = true);

      static void addElementRelativeBeam(
        std::list<Ray> &dest,
        Element *element,
        unsigned int number = 1000,
        Real radius = .5,
        Real azimuth = 0,
        Real elevation = 90,
        Real offX = 0,
        Real offY = 0,
        Real distance = 10,
        uint32_t id = 0,
        bool random = true);

      static void addElementRelativeFocusBeam(
        std::list<Ray> &dest,
        Element *element,
        unsigned int number = 1000,
        Real radius = .5,
        Real fNum = 17.37,
        Real refAperture = 200e-3,
        Real azimuth = 0,
        Real elevation = 90,
        Real offX = 0,
        Real offY = 0,
        Real distance = 10,
        uint32_t id = 0,
        bool random = true);

      static void addFocalPlaneFocusedBeam(
        std::list<Ray> &dest,
        const ReferenceFrame *focalPlane,
        unsigned int number = 1000,
        Real fNum = 17.37,
        Real azimuth = 0,
        Real elevation = 90,
        Real offX = 0,
        Real offY = 0,
        Real distance = 10,
        uint32_t id = 0,
        bool random = true,
        Real offZ = 0);

      static void addBeam(std::list<Ray> &dest, BeamProperties const &);
      
  };
}

#ifndef RZ_NO_HELPER_MACROS
#  define plugElement(parent, type) plug<type>(parent, STRINGIFY(type))
#  define plugElementName(parent, type, name) plug<type>(parent, STRINGIFY(type), name)
#endif // RZ_NO_HELPER_MACROS

#endif // _OMMODEL_H