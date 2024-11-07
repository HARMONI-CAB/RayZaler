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

#include <DataProducts/Scatter.h>
#include <DataProducts/ScatterTree.h>
#include <OpticalElement.h>

using namespace RZ;

/////////////////////////////// ScatterSet ////////////////////////////////////
ScatterSet::ScatterSet(
  uint32_t id,
  std::vector<Real> &locations,
  std::string const &label,
  unsigned stride,
  bool transfer)
{
  size_t i = 0;

  m_tree  = new ScatterTree();
  m_label = label;
  m_id    = id;

  if (transfer) {
    m_tree->setStride(stride);
    m_tree->transfer(locations);
  } else while (i < locations.size()) {
    m_tree->push(locations[i + 0], locations[i + 1]);
    i += stride;
  }

  m_size = i / stride;
}

ScatterSet::ScatterSet(
  uint32_t id,
  std::vector<Real> const &locations,
  std::string const &label,
  unsigned stride)
{
  size_t i = 0;

  m_tree  = new ScatterTree();
  m_label = label;
  m_id    = id;

  while (i < locations.size()) {
    m_tree->push(locations[i + 0], locations[i + 1]);
    i += stride;
  }

  m_size = i / stride;
}


ScatterSet::ScatterSet(
  uint32_t id,
  OpticalSurface const *surface,
  std::string const &label) : ScatterSet(
    id,                       // Id
    surface->locations(),     // 3D vector of locations 
    label,                    // Label
    3)                        // Stride
{
}

ScatterSet::~ScatterSet()
{
  delete m_tree;
}

void
ScatterSet::rebuild()
{
  if (!m_built) {
    m_tree->rebuild();
    m_tree->setFinestScale(2);
    m_built = true;
  }
}

void
ScatterSet::render(ScatterTreeRenderer *renderer) const
{
  renderer->setId(m_id);
  m_tree->render(renderer);
}

std::string const &
ScatterSet::label() const
{
  return m_label;
}

uint32_t
ScatterSet::id() const
{
  return m_id;
}

size_t
ScatterSet::size() const
{
  return m_size;
}

/////////////////////////////// ScatterDataProduct /////////////////////////////
ScatterDataProduct::ScatterDataProduct(std::string const &name)
{
  setProductName(name);
  if (pthread_mutex_init(&m_lock, nullptr) != 0)
    throw std::runtime_error("Failed to create lock");

  m_haveLock = true;
}

ScatterDataProduct::~ScatterDataProduct()
{
  clear();

  if (m_haveLock)
    pthread_mutex_destroy(&m_lock);
}

void
ScatterDataProduct::build()
{
  pthread_mutex_lock(&m_lock);
  for (auto &p: m_setList)
    p->rebuild();
  pthread_mutex_unlock(&m_lock);
}

void
ScatterDataProduct::render(ScatterTreeRenderer *renderer) const
{
  pthread_mutex_lock(&m_lock);
  for (auto &p: m_setList)
    p->render(renderer);
  pthread_mutex_unlock(&m_lock);
}

void
ScatterDataProduct::clear()
{
  for (auto &p: m_setList)
    delete p;
  
  m_setList.clear();
}

size_t
ScatterDataProduct::size() const
{
  return m_points;
}

void
ScatterDataProduct::addSet(ScatterSet *set)
{
  pthread_mutex_lock(&m_lock);
  m_setList.push_back(set);
  m_points += set->size();
  pthread_mutex_unlock(&m_lock);

  discardView();
}

void
ScatterDataProduct::addSurface(
  uint32_t id,
  OpticalSurface const *surface,
  std::string const &label)
{
  addSet(new ScatterSet(id, surface, label));
}

void
ScatterDataProduct::addSurface(
  OpticalSurface const *surface,
  std::string const &label)
{
  addSurface(m_idCount++, surface, label);
}

std::string
ScatterDataProduct::productType() const
{
  return "Scatter";
}

bool
ScatterDataProduct::saveToFile(std::string const &path) const
{
  return false;
}

std::list<ScatterSet *> const &
ScatterDataProduct::dataSets() const
{
  return m_setList;
}
