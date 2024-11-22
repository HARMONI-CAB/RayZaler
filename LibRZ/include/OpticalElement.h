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

#ifndef _OPTICAL_ELEMENT_H
#define _OPTICAL_ELEMENT_H

#include "Element.h"
#include <list>
#include <map>
#include <string>
#include <RayTracingEngine.h>

namespace RZ {
  class RayTransferProcessor;
  class OpticalElement;
  

  struct OpticalSurface {
    std::string                 name;
    const ReferenceFrame       *frame     = nullptr;
    const RayTransferProcessor *processor = nullptr;
    OpticalElement             *parent;

    std::map<uint32_t, RayBeamStatistics> statistics;
    
    mutable std::vector<Ray>    hits;

    // Haha C++
    mutable std::vector<Real>   locationArray;
    mutable std::vector<Real>   directionArray;

    std::vector<Real> &locations() const;
    std::vector<Real> &directions() const;

    void clearCache() const;
    void clearStatistics();
  };

  struct OpticalPath {
    std::list<const OpticalSurface *> m_sequence;
    std::map<std::string, const OpticalSurface *> m_nameToSurface;

    OpticalPath &plug(OpticalElement *, std::string const &name = "");
    void push(const OpticalSurface *);

    const std::vector<Real> &hits(std::string const &name) const;
    const std::vector<Real> &directions(std::string const &name) const;

    inline const OpticalSurface *
    getSurface(std::string const &name) const
    {
      auto it = m_nameToSurface.find(name);

      if (it == m_nameToSurface.cend())
        return nullptr;

      return it->second;
    }

    inline std::list<std::string>
    surfaces() const
    {
      std::list<std::string> list;

      for (auto &p : m_nameToSurface)
        list.push_back(p.first);

      return list;
    }
  };

  class OpticalElement : public Element {
      using Element::Element;
      std::list<OpticalSurface>               m_surfaces; // Owned
      std::list<ReferenceFrame *>             m_surfaceFrames; // Owned
      OpticalPath                             m_internalPath;
      bool                                    m_recordHits = false;

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
      inline bool
      recordHits() const
      {
        return m_recordHits;
      }
      
      virtual OpticalPath opticalPath(std::string const &name = "") const;
      OpticalPath plug(OpticalElement *, std::string const &name = "") const;
      const std::list<const OpticalSurface *> &opticalSurfaces() const;

      const std::vector<Real> &hits(std::string const &name = "") const;
      const std::vector<Real> &directions(std::string const &name = "") const;

      virtual void setRecordHits(bool);
      virtual void clearHits();

      virtual ~OpticalElement();
  };
}

#endif // _OPTICAL_ELEMENT_H
