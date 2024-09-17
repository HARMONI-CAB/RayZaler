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

OpticalPath &
OpticalPath::plug(OpticalElement *element, std::string const &name)
{
  OpticalPath copy = element->opticalPath(name);

  m_sequence.insert(
    m_sequence.end(),
    copy.m_sequence.begin(),
    copy.m_sequence.end());

  return *this;
}

OpticalPath
OpticalElement::opticalPath(std::string const &name) const
{
  OpticalPath path;

  if (name.size() == 0)
    for (auto &p : m_internalPath)
      path.m_sequence.push_back(&p);
  else
    throw std::runtime_error("Unknown optical path `" + name + "'");
  
  return path;
}

OpticalPath
OpticalElement::plug(OpticalElement *newElement, std::string const &name) const
{
  return opticalPath().plug(newElement, name);
}

const std::list<const OpticalSurface *> &
OpticalElement::opticalSurfaces() const
{
  return m_surfaceList;
}

void
OpticalElement::pushOpticalSurface(
  std::string name,
  ReferenceFrame *frame,
  const RayTransferProcessor *proc)
{
  OpticalSurface surface;

  m_surfaceFrames.push_back(frame);

  surface.name      = name;
  surface.frame     = frame;
  surface.processor = proc;
  surface.parent    = this;

  m_internalPath.push_back(surface);

  auto last = &m_internalPath.back();

  m_surfaceList.push_back(last);
}

const std::vector<Real> &
OpticalElement::hits(std::string const &name) const
{
  // You just have to love C++
  const OpticalSurface *surface = m_surfaceList.front();
  
  if (!name.empty()) {
    auto it = m_nameToSurface.find(name);
    if (it == m_nameToSurface.cend())
      throw std::runtime_error("No such optical surface `" + name + "'");
    
    surface = it->second;
  }

  return surface->locations();
}

const std::vector<Real> &
OpticalElement::directions(std::string const &name) const
{
  // You just have to love C++
  const OpticalSurface *surface = m_surfaceList.front();
  
  if (!name.empty()) {
    auto it = m_nameToSurface.find(name);
    if (it == m_nameToSurface.cend())
      throw std::runtime_error("No such optical surface `" + name + "'");
    
    surface = it->second;
  }

  return surface->directions();
}


void
OpticalElement::setRecordHits(bool record)
{
  m_recordHits = record;
}

void
OpticalElement::clearHits()
{
  for (auto &p : m_internalPath)
    p.hits.clear();
}

OpticalElement::OpticalElement(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *parentFrame,
  Element *parent)
  : Element(factory, name, parentFrame, parent)
{
  registerProperty("optical", true);

  ++m_hidden;
}

OpticalElement::~OpticalElement()
{
  for (auto p : m_surfaceFrames)
    delete p;
}
