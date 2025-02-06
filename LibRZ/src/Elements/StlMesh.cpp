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

#include <GL/glew.h>
#include <Elements/StlMesh.h>
#include <Elements/stl_reader.h>
#include <Logger.h>
#include <GenericCompositeModel.h>

using namespace RZ;

RZ_DESCRIBE_ELEMENT(StlMesh, "Arbitrary mesh from a STL file")
{
  property("file",  "",   "Path of the STL mesh");
  property("units", 1e-3, "Physical length of the units of the STL mesh [m]");
}

void
StlMesh::tryOpenModel()
{
  m_haveMesh = false;
  m_coords.clear();
  m_normals.clear();
  m_tris.clear();
  m_solids.clear();

  m_vertices.clear();
  m_vnormals.clear();
  
  std::string actualPath;
  auto model = parentModel();

  actualPath = model == nullptr ? m_path : model->resolveFilePath(m_path);

  Vec3 p1, p2;
  bool first = true;

  try {
    stl_reader::ReadStlFile(
      actualPath.c_str(),
      m_coords,
      m_normals,
      m_tris,
      m_solids);
    
    const size_t numTris = m_tris.size() / 3;
    const size_t numVrtx = m_coords.size() / 3;

    // We start by creating a vertex array with separate vertices for each
    // triangle. The same applies to normals.
    m_vertices.resize(9 * numTris);
    m_vnormals.resize(9 * numTris);

    // And now, for each triangle i
    for (unsigned i = 0; i < numTris; ++i) {
      const Real *normal = &m_normals[3 * i];

      // For each vertex j
      for (unsigned j = 0; j < 3; ++j) {
        unsigned int n = 3 * i + j;
        unsigned int c = m_tris[n];

        if (c >= numVrtx)
          throw std::runtime_error("STL mesh invalid: reference to unknown vertex!");
        
        // Get the vnormals array
        const Real *coord = &m_coords[3 * c];
        Real *vNorm       = &m_vnormals[3 * n];
        Real *vCoord      = &m_vertices[3 * n];
        auto p            = Vec3(coord) * m_units;

        if (first) {
          first = false;
          p1 = p2 = p;
        } else {
          expandBox(p1, p2, p);
        }

        memcpy(vNorm,  normal, 3 * sizeof (Real));
        memcpy(vCoord, coord, 3 * sizeof (Real));

        // Update triangle index
        m_tris[n] = n;
      }
    }

    m_coords.clear();
    m_normals.clear();
    m_solids.clear();

    setBoundingBox(p1, p2);
    m_haveMesh = true;
  } catch (std::exception& e) {
    RZError(
      "%s: cannot load STL model from `%s': %s\n",
      name().c_str(),
      m_path.c_str(),
      e.what());
  }
}

bool
StlMesh::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "file") {
    std::string newPath = std::get<std::string>(value);
    if (m_path != newPath) {
      m_path = newPath;
      tryOpenModel();
    }
  } else if (name == "units") {
    m_units = value;
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}

StlMesh::StlMesh(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : Element(factory, name, frame, parent)
{
}

StlMesh::~StlMesh()
{
}

void
StlMesh::enterOpenGL()
{
 
}

void
StlMesh::renderOpenGL()
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  if (m_haveMesh) {
    material("main");
    glPushMatrix();
    glScalef(m_units, m_units, m_units);
    glVertexPointer(3, GL_DOUBLE,   3 * sizeof(Real), m_vertices.data());
    glNormalPointer(GL_DOUBLE,      3 * sizeof(Real), m_vnormals.data());
    glDrawElements(GL_TRIANGLES, m_tris.size(), GL_UNSIGNED_INT, m_tris.data());
    glPopMatrix();
  }

  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopAttrib();
}
