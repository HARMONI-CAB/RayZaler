#ifndef _OPTICAL_ELEMENT_H
#define _OPTICAL_ELEMENT_H

#include "Element.h"
#include <list>
#include <map>
#include <string>

namespace RZ {
  class RayTransferProcessor;

  class OpticalElement;
  
  struct OpticalSurface {
    std::string                 name;
    const ReferenceFrame       *frame     = nullptr;
    const RayTransferProcessor *processor = nullptr;
    OpticalElement             *parent;
  };

  struct OpticalPath {
    std::list<OpticalSurface>   m_sequence;

    OpticalPath &plug(OpticalElement *, std::string const &name = "");
  };

  class OpticalElement : public Element {
      using Element::Element;
      std::list<ReferenceFrame *> m_surfaceFrames; // Owned
      std::list<OpticalSurface>   m_internalPath;
      std::list<const OpticalSurface *> m_surfaceList;
      std::map<std::string, OpticalSurface *> m_nameToSurface;

    protected:
      void pushOpticalSurface(
        std::string,
        ReferenceFrame *,
        const RayTransferProcessor *);
      OpticalElement(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
    public:
      virtual OpticalPath opticalPath(std::string const &name = "") const;
      OpticalPath plug(OpticalElement *, std::string const &name = "") const;
      const std::list<const OpticalSurface *> &opticalSurfaces() const;
      virtual ~OpticalElement();
      
  };
}

#endif // _OPTICAL_ELEMENT_H
