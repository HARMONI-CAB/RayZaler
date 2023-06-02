#ifndef TRANSLATEDFRAME_H
#define TRANSLATEDFRAME_H

#include <ReferenceFrame.h>

namespace RZ {
  class TranslatedFrame : public ReferenceFrame {
      Vec3 m_distance;
      int m_distanceIndex = -1;
      ReferenceFrame *m_parent;

    protected:
      void recalculateFrame();

    public:
      TranslatedFrame(
        std::string const &name,
        ReferenceFrame *parent,
        Vec3 const &distance);

      void setDistanceX(Real);
      void setDistanceY(Real);
      void setDistanceZ(Real);

      void setDistance(Vec3 const &);
  };
}

#endif // TRANSLATEDFRAME_H
