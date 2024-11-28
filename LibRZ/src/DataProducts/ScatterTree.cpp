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

#include <DataProducts/ScatterTree.h>
#include <cassert>

using namespace RZ;

ScatterTreeRenderer::ScatterTreeRenderer()
{
  
}

ScatterTreeRenderer::~ScatterTreeRenderer()
{

}

void
ScatterTreeRenderer::setId(uint32_t)
{
  // Ignored by default
}

ScatterVec::ScatterVec() : ScatterVec(0) { }
ScatterVec::ScatterVec(double val) : ScatterVec(val, val) { }
ScatterVec::ScatterVec(int val) : ScatterVec(static_cast<double>(val)) { }
ScatterTree::ScatterTree() { }

ScatterVec &
ScatterVec::operator/=(double k)
{
  x /= k;
  y /= k;

  return *this;
}

ScatterVec &
ScatterVec::operator*=(double k)
{
  x *= k;
  y *= k;

  return *this;
}

ScatterVec &
ScatterVec::operator+=(ScatterVec const &other)
{
  x += other.x;
  y += other.y;

  return *this;
}

ScatterVec &
ScatterVec::operator-=(ScatterVec const &other)
{
  x -= other.x;
  y -= other.y;

  return *this;
}

ScatterVec
ScatterVec::operator+(ScatterVec const &other) const
{
  return ScatterVec(x + other.x, y + other.y);
}

ScatterVec
ScatterVec::operator-(ScatterVec const &other) const
{
  return ScatterVec(x - other.x, y - other.y);
}

ScatterVec::ScatterVec(double x, double y)
{
  this->x = x;
  this->y = y;
}


ScatterTreeNode *
ScatterTree::makeNode()
{
  m_alloc.push_back(ScatterTreeNode());
  return &m_alloc.back();
}

void
ScatterTree::push(double x, double y)
{
  size_t last = m_points.size() / m_stride;

  m_points.resize((last + 1) * m_stride);

  m_points[m_stride * last + 0] = x;
  m_points[m_stride * last + 1] = y;
}


//
// The algorithm is as follows:
// 1. Start by moving all points to root node, and set current = root
// 2. In current, find min, max and cog, and set nelem
// 3. Now, if there are more elements than threshold:
//    3.1 Take al points and divide them into quadrants around cog
//    3.2 For each quadrant, repeat from 1.
//

void
ScatterTree::buildNode(ScatterTreeNode *node)
{
  unsigned int numLeaves = 0;
  
  auto &cog         = node->cog;
  auto &topLeft     = node->topLeft;
  auto &bottomRight = node->bottomRight;
  

  node->nElem = node->unplaced.size();
  bool doSplit = node->nElem > m_splitThreshold;

  cog = sumPrecise(node->unplaced.data(), node->nElem);
  cog /= node->nElem;

  topLeft     = cog;
  bottomRight = cog;

  auto it = node->unplaced.begin();

  while (it != node->unplaced.end()) {
    auto current = it++;
    auto &p = *current;

    if (p.x < topLeft.x)
      topLeft.x = p.x;

    if (p.y < topLeft.y)
      topLeft.y = p.y;

    if (p.x > bottomRight.x)
      bottomRight.x = p.x;

    if (p.y > bottomRight.y)
      bottomRight.y = p.y;

    // If the list is too big, split it into quadrants and repeat.
    if (doSplit) {
      int xIndex = p.x < node->cog.x ? 0 : 1;
      int yIndex = p.y < node->cog.y ? 0 : 1;
      auto &leaf = node->leaves[yIndex][xIndex];

      if (leaf == nullptr) {
        leaf = makeNode();
        ++numLeaves;
      }

      leaf->unplaced.push_back(p);
    }
  }

  // There is a degenerate case in which all points are always the same.
  // We must abort the recursive split in this case.

  if (numLeaves > 1) {
    for (auto i = 0; i < 2; ++i)
      for (auto j = 0; j < 2; ++j)
        if (node->leaves[j][i] != nullptr)
          buildNode(node->leaves[j][i]);
  }
}

static inline int
loc2px(double x, double min, double res)
{
  return int ((x - min) / res);
}

void
ScatterTree::renderNode(
    const ScatterTreeNode *node,
    ScatterTreeRenderer *renderer,
    ScatterVec const &min,
    ScatterVec const &max,
    ScatterVec const &res)
{
  ScatterVec boundingBox = node->bottomRight - node->topLeft;
  
  // Check bounding box resolution. If the bounding box is small
  // compared to the resolution element, just notify a pixel the size
  // of this node.

  if (boundingBox.x <= m_finestScale * res.x
      && boundingBox.y <= m_finestScale * res.y) {
    auto &cog = node->cog;

    if (cog.inRange(min, max))
      renderer->render(
        loc2px(cog.x, min.x, res.x),
        loc2px(cog.y, min.y, res.y),
        node->nElem);
  } else {
    // When we hit a node with a non-empty list, plot the points individually
    for (auto &p : node->unplaced)
      if (p.inRange(min, max))
        renderer->render(
          loc2px(p.x, min.x, res.x),
          loc2px(p.y, min.y, res.y),
          1);

    // If there are finer views, go ahead
    for (auto i = 0; i < 2; ++i)
      for (auto j = 0; j < 2; ++j)
        if (node->leaves[j][i] != nullptr) {
          auto leaf = node->leaves[j][i];
          if (leaf->bottomRight.x < min.x 
             || leaf->bottomRight.y < min.y 
             || max.x < leaf->topLeft.x 
             || max.y < leaf->topLeft.y)
             continue;
          
          renderNode(leaf, renderer, min, max, res);
        }
  }
}

void
ScatterTree::setSplitThreshold(unsigned int threshold)
{
  assert(threshold > 0);
  m_splitThreshold = threshold;
}

void
ScatterTree::render(ScatterTreeRenderer *renderer)
{
  if (m_root != nullptr)
    renderNode(
      m_root,
      renderer,
      renderer->topLeft(),
      renderer->bottomRight(),
      renderer->resolution());
}

void
ScatterTree::render(
    ScatterTreeRenderer *renderer,
    ScatterVec const &min,
    ScatterVec const &max)
{
  if (m_root != nullptr)
    renderNode(m_root, renderer, min, max, renderer->resolution());
}

void
ScatterTree::rebuild()
{
  size_t pointCount = m_points.size() / m_stride;
  size_t i;

  m_alloc.clear();
  m_root = nullptr;

  if (m_points.empty())
    return;

  m_root = makeNode();

  m_root->unplaced.resize(pointCount);

  for (i = 0; i < pointCount; ++i)
    m_root->unplaced[i] = 
      std::move(*reinterpret_cast<ScatterVec *>(m_points.data() + i * m_stride));

  buildNode(m_root);
}

void
ScatterTree::setFinestScale(double scale)
{
  m_finestScale = scale;
}

void
ScatterTree::setStride(unsigned int stride)
{
  if (m_stride != stride) {
    m_stride = stride;
    m_points.clear();
  }
}

void
ScatterTree::transfer(std::vector<double> &data)
{
  std::swap(data, m_points);
}
