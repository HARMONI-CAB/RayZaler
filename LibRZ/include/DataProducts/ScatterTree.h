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

#ifndef _SCATTER_TREE_H
#define _SCATTER_TREE_H

#include <list>
#include <vector>
#include <Helpers.h>

namespace RZ {
  struct ScatterVec {
    union {
      struct {
        double x;
        double y;
      };

      double coord[2];
    };

    ScatterVec();
    ScatterVec(double x, double y);
    ScatterVec(double);
    ScatterVec(int);

    ScatterVec &operator+=(ScatterVec const &);
    ScatterVec &operator-=(ScatterVec const &);
    ScatterVec &operator/=(double);
    ScatterVec &operator*=(double);

    ScatterVec operator+(ScatterVec const &) const;
    ScatterVec operator-(ScatterVec const &) const;

    inline bool
    inRange(const ScatterVec &min, const ScatterVec &max) const
    {
      return min.x <= x && x < max.x && min.y <= y && y < max.y;
    }
  };

  template<>
  struct is_real<ScatterVec> {
    static constexpr bool value = true;
  };

  class ScatterTree;

  struct ScatterTreeNode {
    ScatterVec              cog;
    ScatterVec              topLeft;
    ScatterVec              bottomRight;
    unsigned int            nElem;

    ScatterTreeNode        *leaves[2][2];
    std::vector<ScatterVec> unplaced;
  };

  class ScatterTreeRenderer {
  public:
    virtual ScatterVec resolution() const = 0;
    virtual ScatterVec topLeft() const = 0;
    virtual ScatterVec bottomRight() const = 0;
    virtual void setId(uint32_t);
    virtual void render(int x, int y, unsigned int count) = 0;

    ScatterTreeRenderer();
    virtual ~ScatterTreeRenderer();
  };

  class ScatterTree {
    unsigned int               m_stride = 2;
    ScatterTreeNode           *m_root = nullptr;
    std::list<ScatterTreeNode> m_alloc;
    std::vector<double>        m_points;
    double                     m_finestScale = 1;
    unsigned int               m_splitThreshold = 100;

    ScatterTreeNode *makeNode();

    void buildNode(ScatterTreeNode *);
    void renderNode(
      const ScatterTreeNode *,
      ScatterTreeRenderer *,
      ScatterVec const &min,
      ScatterVec const &max,
      ScatterVec const &res);
    
  public:
    ScatterTree();

    void push(double x, double y);
    void setStride(unsigned int stride);
    void transfer(std::vector<double> &data);
    void rebuild();
    void render(ScatterTreeRenderer *);
    void render(
        ScatterTreeRenderer *,
        ScatterVec const &,
        ScatterVec const &);
    void setSplitThreshold(unsigned int);
    void setFinestScale(double);
  };
}

#endif // _SCATTER_TREE_H
