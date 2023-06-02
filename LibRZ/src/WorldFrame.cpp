#include <WorldFrame.h>

using namespace RZ;

WorldFrame::WorldFrame(std::string const &name) : ReferenceFrame(name)
{
}

void
WorldFrame::recalculateFrame()
{
  // Sane orientation
  setOrientation(Matrix3::eye());

  // Reasonable center
  setCenter(Vec3::zero());
}
