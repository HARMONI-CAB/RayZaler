#ifndef WORLDFRAME_H
#define WORLDFRAME_H

#include <ReferenceFrame.h>

namespace RZ {
  class WorldFrame : public ReferenceFrame {
      ReferenceFrame *m_parent = nullptr;

    protected:
      void recalculateFrame();

    public:
      WorldFrame(std::string const &name);
      void linkParent(ReferenceFrame *frame);
  };
}

#endif // WORLDFRAME_H
