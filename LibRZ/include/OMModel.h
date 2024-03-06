#ifndef _OMMODEL_H
#define _OMMODEL_H

#include <Vector.h>
#include <WorldFrame.h>
#include <ReferenceFrame.h>
#include <Element.h>
#include <Singleton.h>
#include <OpticalElement.h>
#include <Detector.h>
#include <Random.h>
#include <list>
#include <map>

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define RZ_DEFAULT_CCD_RESOLUTION 1024
#define RZ_DEFAULT_CCD_WIDTH      5e-2

namespace RZ {
  class OMModel;
  class GenericCompositeModel;
  
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
      std::list<Ray>  m_intermediateRays;
      struct timeval  m_lastTick;

      // Random state for Diffraction calculations
      ExprRandomState m_randState;

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
      Element *autoRegisterElement(Element *);

      inline ReferenceFrame *
      world() const
      {
        return m_world;
      }
      
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
      Element *beam() const;
      void clearBeam();
      
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

      // Convenience methods (fast enumeration)
      std::list<Element *> const &elementList() const;

      // Lookup methods
      ReferenceFrame *lookupReferenceFrame(std::string const &) const;
      Element *lookupElement(std::string const &) const;
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
        Real distance = 10);

      static void addElementRelativeBeam(
        std::list<Ray> &dest,
        Element *element,
        unsigned int number = 1000,
        Real radius = .5,
        Real azimuth = 0,
        Real elevation = 90,
        Real offX = 0,
        Real offY = 0,
        Real distance = 10);

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
        Real distance = 10);

      static void addFocalPlaneFocusedBeam(
        std::list<Ray> &dest,
        const ReferenceFrame *focalPlane,
        unsigned int number = 1000,
        Real fNum = 17.37,
        Real azimuth = 0,
        Real elevation = 90,
        Real offX = 0,
        Real offY = 0,
        Real distance = 10);
  };
}

#ifndef RZ_NO_HELPER_MACROS
#  define plugElement(parent, type) plug<type>(parent, STRINGIFY(type))
#  define plugElementName(parent, type, name) plug<type>(parent, STRINGIFY(type), name)
#endif // RZ_NO_HELPER_MACROS

#endif // _OMMODEL_H