#include <ReferenceFrame.h>

using namespace RZ;

int
ReferenceFrame::replaceAxis(std::string const &name, Vec3 const &relative)
{
  NamedVector *vec;
  int index;

  if (m_nameToAxis.find(name) == m_nameToAxis.end())
    return -1;

  index         = m_nameToAxis[name];
  vec           = &m_axes[index];
  vec->relative = relative;

  return index;
}

int
ReferenceFrame::replacePoint(std::string const &name, Point3 const &relative)
{
  NamedVector *vec;
  int index;

  if (m_nameToPoint.find(name) == m_nameToPoint.end())
    return -1;

  index         = m_nameToPoint[name];
  vec           = &m_points[index];
  vec->relative = relative;

  return index;
}

int
ReferenceFrame::addAxis(std::string const &name, Vec3 const &relative)
{
  NamedVector vec;
  int index;

  if ((index = replaceAxis(name, relative)) != -1)
    return index;
  
  vec.name     = name;
  vec.relative = relative;

  index = static_cast<int>(m_axes.size());
  m_axes.push_back(vec);

  m_nameToAxis[name] = index;
  m_nameToAxisCache.clear();
  m_nameToAxisCache[name] = &m_axes[index].absolute;

  return index;
}

int
ReferenceFrame::addPoint(std::string const &name, Vec3 const &relative)
{
  NamedVector point;
  int index;

  if ((index = replacePoint(name, relative)) != -1)
    return index;

  point.name     = name;
  point.relative = relative;

  index = static_cast<int>(m_points.size());
  m_points.push_back(point);

  m_nameToPoint[name] = index;
  m_nameToPointCache.clear();
  m_nameToPointCache[name] = &m_points[index].absolute;

  return index;
}

void
ReferenceFrame::addChild(ReferenceFrame *child)
{
  m_children.push_back(child);
}

void
ReferenceFrame::recalculateChildren()
{
  for (auto p : m_children)
    p->recalculate();
}

const Matrix3 &
ReferenceFrame::getOrientation() const
{
  return m_orientation;
}

const Vec3 &
ReferenceFrame::getCenter() const
{
  return m_center;
}

int
ReferenceFrame::getAxisIndex(std::string const &name) const
{
  auto p = m_nameToAxis.find(name);

  if (p == m_nameToAxis.end())
    return -1;

  return p->second;
}

int
ReferenceFrame::getPointIndex(std::string const &name) const
{
  auto p = m_nameToPoint.find(name);

  if (p == m_nameToPoint.end())
    return -1;

  return p->second;
}

const Vec3 *
ReferenceFrame::getAxis(std::string const &name)
{
  auto cached = m_nameToAxisCache.find(name);

  // Find axis in the axis cache
  if (cached == m_nameToAxisCache.end()) {
    int index = getAxisIndex(name);

    if (index == -1)
      return nullptr;
    
    m_nameToAxisCache[name] = &m_axes[index].absolute;
    return &m_axes[index].absolute;
  }

  return cached->second;
}

const Vec3 *
ReferenceFrame::getPoint(std::string const &name)
{
  auto cached = m_nameToPointCache.find(name);

  // Find axis in the axis cache
  if (cached == m_nameToPointCache.end()) {
    int index = getPointIndex(name);

    if (index == -1)
      return nullptr;
    
    m_nameToPointCache[name] = &m_points[index].absolute;
    return &m_points[index].absolute;
  }

  return cached->second;
}


const Vec3 *
ReferenceFrame::getAxis(std::string const &name) const
{
  auto cached = m_nameToAxisCache.find(name);

  // Find axis in the axis cache
  if (cached == m_nameToAxisCache.end()) {
    int index = getAxisIndex(name);

    if (index == -1)
      return nullptr;
    
    return &m_axes[index].absolute;
  }

  return cached->second;
}

const Vec3 *
ReferenceFrame::getPoint(std::string const &name) const
{
  auto cached = m_nameToPointCache.find(name);

  // Find axis in the axis cache
  if (cached == m_nameToPointCache.end()) {
    int index = getPointIndex(name);

    if (index == -1)
      return nullptr;
    
    return &m_points[index].absolute;
  }

  return cached->second;
}

const Vec3 *
ReferenceFrame::getAxis(int index) const
{
  if (index < 0 || index >= m_axes.size())
    return nullptr;

  return &m_axes[index].absolute;
}

const Vec3 *
ReferenceFrame::getPoint(int index) const
{
  if (index < 0 || index >= m_points.size())
    return nullptr;

  return &m_points[index].absolute;
}

Vec3 *
ReferenceFrame::getAxis(int index)
{
  if (index < 0 || index >= m_axes.size())
    return nullptr;

  return &m_axes[index].absolute;
}

Vec3 *
ReferenceFrame::getPoint(int index)
{
  if (index < 0 || index >= m_points.size())
    return nullptr;

  return &m_points[index].absolute;
}

void
ReferenceFrame::recalculateVectors()
{
  size_t i;

  // Axes are the easiest ones to transform
  for (i = 0; i < m_axes.size(); ++i)
    m_axes[i].absolute = m_orientation * m_axes[i].relative;

  // Points are a bit trickier. While they are defined as a displacement
  // with respect to the reference frame's center, they need to be displaced
  // by the absolute center
  for (i = 0; i < m_points.size(); ++i)
    m_points[i].absolute = m_orientation * m_points[i].relative + m_center;
}

void
ReferenceFrame::recalculate()
{
  recalculateFrame();

  // Post condition: we have m_center and m_orientation
  m_calculated = true;

  recalculateVectors();
  recalculateChildren();
}

void
ReferenceFrame::setCenter(Vec3 const &vec)
{
  m_center = vec;
}

void
ReferenceFrame::setOrientation(Matrix3 const &orientation)
{
  m_orientation = orientation;
}
      

ReferenceFrame::ReferenceFrame(std::string const &name)
{
  m_name = name;

  addAxis("x", Vec3::eX());
  addAxis("y", Vec3::eY());
  addAxis("z", Vec3::eZ());

  addPoint("center", Vec3::zero());
}

ReferenceFrame::ReferenceFrame(std::string const &name, ReferenceFrame *parent)
: ReferenceFrame(name)
{
  m_parent   = parent;
  parent->addChild(this);
}

ReferenceFrame::~ReferenceFrame()
{
  
}