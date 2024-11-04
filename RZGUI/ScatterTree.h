#ifndef _SCATTER_TREE_H
#define _SCATTER_TREE_H

#include <list>
#include <vector>
#include <Helpers.h>

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
struct RZ::is_real<ScatterVec> {
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

  virtual void render(int x, int y, unsigned int count) = 0;

  virtual ~ScatterTreeRenderer();
};

class ScatterTree {
  ScatterTreeNode           *m_root = nullptr;
  std::list<ScatterTreeNode> m_alloc;
  std::vector<ScatterVec>    m_points;
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
  void rebuild();
  void render(ScatterTreeRenderer *);
  void render(
      ScatterTreeRenderer *,
      ScatterVec const &,
      ScatterVec const &);
  void setSplitThreshold(unsigned int);
  void setFinestScale(double);
};

#endif // _SCATTER_TREE_H
