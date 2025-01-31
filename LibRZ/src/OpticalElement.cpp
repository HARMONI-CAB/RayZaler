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

#include <OpticalElement.h>

using namespace RZ;

RZ_DESCRIBE_ABSTRACT_ELEMENT(OpticalElement, "Generic optical element")
{
  hiddenProperty("optical", true, "The element is optical");
}

std::vector<Real> &
OpticalSurface::locations() const
{
  size_t expectedSize = 3 * hits.size();

  if (locationArray.size() != expectedSize) {
    locationArray.resize(expectedSize);

    for (size_t i = 0; i < hits.size(); ++i)
      frame->toRelative(hits[i].origin).copyToArray(locationArray.data() + 3 * i);
  }

  return locationArray;
}

std::vector<Real> &
OpticalSurface::directions() const
{
  size_t expectedSize = 3 * hits.size();

  if (directionArray.size() != expectedSize) {
    directionArray.resize(expectedSize);

    for (size_t i = 0; i < hits.size(); ++i)
      frame->toRelativeVec(hits[i].direction).copyToArray(directionArray.data() + 3 * i);
  }

  return directionArray;
}

void
OpticalSurface::clearCache() const
{
  locationArray.clear();
  directionArray.clear();
}

void
OpticalSurface::clearStatistics()
{
  statistics.clear();
}

///////////////////////////// Optical Path API /////////////////////////////////
OpticalPath &
OpticalPath::plug(OpticalElement *element, std::string const &name)
{
  for (auto &p : element->opticalPath(name).m_sequence)
    push(p);

  return *this;
}

void
OpticalPath::push(const OpticalSurface *surface)
{
  m_sequence.push_back(surface);
  m_nameToSurface[surface->name] = surface;
}

const std::vector<Real> &
OpticalPath::hits(std::string const &name) const
{
  // You just have to love C++
  const OpticalSurface *surface = m_sequence.front();

  if (!name.empty()) {
    auto it = m_nameToSurface.find(name);
    if (it == m_nameToSurface.cend())
      throw std::runtime_error("No such optical surface `" + name + "'");
    
    surface = it->second;
  }

  return surface->locations();
}

const std::vector<Real> &
OpticalPath::directions(std::string const &name) const
{
  // You just have to love C++
  const OpticalSurface *surface = m_sequence.front();
  
  if (!name.empty()) {
    auto it = m_nameToSurface.find(name);
    if (it == m_nameToSurface.cend())
      throw std::runtime_error("No such optical surface `" + name + "'");
    
    surface = it->second;
  }

  return surface->directions();
}

////////////////////////// Optical Element ////////////////////////////////////
OpticalPath
OpticalElement::opticalPath(std::string const &name) const
{
  if (name.empty())
    return m_internalPath;
  else
    throw std::runtime_error("Unknown optical path `" + name + "'");
}

OpticalPath
OpticalElement::plug(OpticalElement *newElement, std::string const &name) const
{
  return opticalPath().plug(newElement, name);
}

const std::list<const OpticalSurface *> &
OpticalElement::opticalSurfaces() const
{
  return m_internalPath.m_sequence;
}

void
OpticalElement::pushOpticalSurface(
  std::string name,
  ReferenceFrame *frame,
  const RayTransferProcessor *proc)
{
  OpticalSurface surface;

  // Creation of the optical surface
  m_surfaceFrames.push_back(frame);

  surface.name      = name;
  surface.frame     = frame;
  surface.processor = proc;
  surface.parent    = this;

  m_surfaces.push_back(surface);

  // Insertion at the end of this element's path
  auto last = &m_surfaces.back();
  m_internalPath.push(last);
}

const std::vector<Real> &
OpticalElement::hits(std::string const &name) const
{
  return opticalPath().hits(name);
}

const std::vector<Real> &
OpticalElement::directions(std::string const &name) const
{
  return opticalPath().directions(name);
}

void
OpticalElement::setRecordHits(bool record)
{
  m_recordHits = record;
}

void
OpticalElement::clearHits()
{
  for (auto &p : m_surfaces)
    p.hits.clear();
}

OpticalElement::OpticalElement(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *parentFrame,
  Element *parent)
  : Element(factory, name, parentFrame, parent)
{
}

OpticalElement::~OpticalElement()
{
  for (auto p : m_surfaceFrames)
    delete p;
}
