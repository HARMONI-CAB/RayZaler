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

#ifndef _STL_MESH_H
#define _STL_MESH_H

#include <OpticalElement.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class StlMesh : public Element {
      std::string m_path;
      std::vector<Real>         m_coords,   m_normals;
      std::vector<unsigned int> m_tris,     m_solids;
      std::vector<Real>         m_vertices, m_vnormals;

      Real        m_units    = 1e-3;
      bool        m_haveMesh = false;

      void rescaleVertices();
      void tryOpenModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      StlMesh(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~StlMesh() override;

      virtual void enterOpenGL() override;
      virtual void renderOpenGL() override;
  };

  RZ_DECLARE_ELEMENT(StlMesh);
}

#endif // _STL_MESH_H
