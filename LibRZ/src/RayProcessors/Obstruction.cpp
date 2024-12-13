#include <RayProcessors/Obstruction.h>
#include <ReferenceFrame.h>
#include <Surfaces/Circular.h>

using namespace RZ;

ObstructionProcessor::ObstructionProcessor()
{
  setReversible(true);
  setSurfaceShape(new CircularFlatSurface(m_radius));
  surfaceShape<CircularFlatSurface>()->setObstruction(true);
}

std::string
ObstructionProcessor::name() const
{
  return "Obstruction";
}

void
ObstructionProcessor::setRadius(Real R)
{
  surfaceShape<CircularFlatSurface>()->setRadius(R);
  m_radius = R;
}

void
ObstructionProcessor::setObstructionMap(
  Real width,
  Real height,
  std::vector<Real> const &map,
  unsigned int cols,
  unsigned int rows,
  unsigned int stride)
{
  m_hx             = width  / cols;
  m_hy             = height / rows;

  m_cols           = cols;
  m_rows           = rows;
  m_stride         = stride;

  if (cols * rows > 0)
    m_obsMapPtr = &map;
  else
    m_obsMapPtr = nullptr;
}

void
ObstructionProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Vec3 center  = plane->getCenter();
  Vec3 tX      = plane->eX();
  Vec3 tY      = plane->eY();
  Real Rsq     = m_radius * m_radius;

  if (m_obsMapPtr != nullptr) {
    std::vector<Real> const &map = *m_obsMapPtr;

    for (i = 0; i < count; ++i) {
      Vec3 coord  = Vec3(beam.destinations + 3 * i) - plane->getCenter();
      Real coordX = coord * tX;
      Real coordY = coord * tY;

      int  pixI   = +floor(coordX / m_hx) + m_cols / 2;
      int  pixJ   = -floor(coordY / m_hy) + m_rows / 2;

      if (pixI >= 0 && pixI < m_cols && pixJ >= 0 && pixJ < m_rows) {
        if (map[pixI + pixJ * m_stride] > 0.5) {
          beam.prune(i);
          continue;
        }
      }
      
      beam.interceptDone(i);
    }
  } else {
    for (i = 0; i < count; ++i) {
      Vec3 coord  = Vec3(beam.destinations + 3 * i) - plane->getCenter();
      Real coordX = coord * tX;
      Real coordY = coord * tY;

      if (coordX * coordX + coordY * coordY <= Rsq)
        beam.prune(i);
      else
        beam.interceptDone(i);
    }
  }
}
