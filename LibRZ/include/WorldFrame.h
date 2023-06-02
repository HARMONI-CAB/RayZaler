#ifndef WORLDFRAME_H
#define WORLDFRAME_H

#include <ReferenceFrame.h>

namespace RZ {
  class WorldFrame : public ReferenceFrame {
    protected:
      void recalculateFrame();

    public:
      WorldFrame(std::string const &name);
  };
}

#endif // WORLDFRAME_H
