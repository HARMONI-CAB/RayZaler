#ifndef REFERENCEFRAME_H
#define REFERENCEFRAME_H

#include <string>
#include <Vector.h>
#include <Matrix.h>
#include <map>
#include <vector>
#include <list>

#define RZ_REF_FRAME_ROTATION_ID    0x1000
#define RZ_REF_FRAME_TRANSLATION_ID 0x1001

namespace RZ{
  struct NamedVector {
    std::string name;
    Vec3        relative; // Relative to the owner
    Vec3        absolute; // Global
  };

  //
  // The global center and orientation are mandatory. After recalculate(), these
  // must be available. How the global center and orientation is calculated is
  // completely upon the implementaion of the ReferenceFrame. For instance:
  //
  // - We may have a GlobalFrame. The center is always (0, 0, 0) and its
  //   orientation is the identity.
  // - We may have a RotatedFrame. This would have a pointer to its base frame.
  //   The implementation of recalculate() would look up the center and rotation
  //   axis of its parent, and calculate the new center and orientation. Its
  //   center would be that of the parent but rotated around the rotation center.
  //   The same would apply to the orientation matrix.
  // - We may also have a CrossProduct frame. This would be calculated from
  //   two previous vectors, etc. In this case, we would have either one
  //   or two dependencies. Maybe assume two different parents?
  //

  class ReferenceFrame {
      std::string m_name;
      Vec3        m_center;      // Global, calculated
      Matrix3     m_orientation; // Global, calculated
      bool        m_calculated = false;
      ReferenceFrame *m_parent = nullptr;
      
      std::vector<NamedVector> m_axes;
      std::vector<NamedVector> m_points;

      std::map<std::string, int> m_nameToAxis;
      std::map<std::string, int> m_nameToPoint;

      // When adding axes and points, these caches get cleared.
      std::map<std::string, Vec3 *> m_nameToAxisCache;
      std::map<std::string, Vec3 *> m_nameToPointCache;

      std::list<ReferenceFrame *> m_children; // Dependents

      void recalculateVectors();

    protected:
      uint32_t m_typeId = 0;

      ReferenceFrame(std::string const &);
      ReferenceFrame(std::string const &, ReferenceFrame *);

      void addChild(ReferenceFrame *);
      void recalculateChildren();

      virtual void recalculateFrame() = 0;

      void setCenter(Vec3 const &);
      void setOrientation(Matrix3 const &);
      
    public:
      // These are added by the dependants or elements. Let us say that
      // this frame was used to create a rotated frame around certain axis
      // defined in this frame. In this case, the dependent would have
      // called addAxis("new_frame.rot_axis", ...) and addPoint("new_frame.rot_center")
      // upon us. This axis and point would belong to us, and we would be in
      // charge not only of recalculating them, but on calling recalculate()
      // on the children that needed this.
      inline bool
      isCalculated() const
      {
        return m_calculated;
      }

      inline ReferenceFrame *
      parent() const 
      {
        return m_parent;
      }

      inline std::string
      name() const
      {
        return m_name;
      }
      
      inline Vec3
      eX() const
      {
        return m_orientation.rows[0];
      }

      inline Vec3
      eY() const
      {
        return m_orientation.rows[1];
      }

      inline Vec3
      eZ() const
      {
        return m_orientation.rows[2];
      }

      inline uint32_t
      typeId() const
      {
        return m_typeId;
      }
      
      int replaceAxis(std::string const &, Vec3 const &);
      int replacePoint(std::string const &, Point3 const &);

      int addAxis(std::string const &, Vec3 const &);  // Relative (to this)
      int addPoint(std::string const &, Point3 const &); // Relative (to this)

      void recalculate();                       // Recalculate this and all children
      const Matrix3 &getOrientation() const;    // Global
      const Point3 &getCenter() const;            // Global

      int getAxisIndex(std::string const &) const;
      int getPointIndex(std::string const &) const;

      const Vec3 *getAxis(std::string const &);        // Global
      const Point3 *getPoint(std::string const &);       // Global

      const Vec3 *getAxis(std::string const &) const;  // Global
      const Point3 *getPoint(std::string const &) const; // Global

      const Vec3 *getAxis(int) const;           // Global
      const Point3 *getPoint(int) const;          // Global
      
      Vec3 *getAxis(int);           // Global
      Point3 *getPoint(int);          // Global
  };
}

#endif // REFERENCEFRAME_H
