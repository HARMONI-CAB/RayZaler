#ifndef _STL_MESH_H
#define _STL_MESH_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class StlMesh : public OpticalElement {
      std::string m_path;
      std::vector<Real>         m_coords, m_normals;
      std::vector<unsigned int> m_tris,   m_solids;
      std::vector<Real>         m_vertices, m_vnormals;
      
      bool        m_haveMesh = false;

      void tryOpenModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      StlMesh(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~StlMesh();

      virtual void enterOpenGL() override;
      virtual void renderOpenGL() override;
  };

  class StlMeshFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _STL_MESH_H
