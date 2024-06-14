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

#include <GLModel.h>
#include <GLHelpers.h>
#include <Element.h>
#include <OpticalElement.h>
#include <GenericAperture.h>
#include <RayProcessors.h>

using namespace RZ;

void
GLModel::configureLighting()
{
  GLVectorStorage vec;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glLightfv(GL_LIGHT0, GL_AMBIENT,  vec.get(1.0, 1.0, 1.0));
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  vec.get(1.0, 1.0, 1.0));
  glLightfv(GL_LIGHT0, GL_SPECULAR, vec.get(1.0, 1.0, 1.0));
  glLightfv(GL_LIGHT0, GL_POSITION, vec.get(1.0, 5.0, 5.0));
  glEnable(GL_LIGHT0);
  
  glLightfv(GL_LIGHT1, GL_AMBIENT,  vec.get(0.1, 0.1, 0.1));
  glLightfv(GL_LIGHT1, GL_DIFFUSE,  vec.get(.5, .5, .5));
  glLightfv(GL_LIGHT1, GL_SPECULAR, vec.get(.5, .5, .5));
  glLightfv(GL_LIGHT1, GL_POSITION, vec.get(1.0, 1.0, 50.0));
  glEnable(GL_LIGHT1);
  
  glShadeModel(GL_SMOOTH);
  glCullFace(GL_BACK);
}

void
GLModel::setEventListener(GLModelEventListener *listener)
{
  m_listener = listener;
}

void
GLModel::tick()
{
  if (m_listener != nullptr)
    m_listener->tick();
}

void
GLModel::updateRefMatrix()
{
  glGetFloatv(GL_MODELVIEW_MATRIX, m_refMatrix);
}

void
GLModel::setApertureColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  GLfloat color[4] = {r, g, b, a};

  setApertureColor(color);
}

void
GLModel::setApertureColor(const GLfloat *rgba)
{
  memcpy(m_apertureColor, rgba, 4 * sizeof(GLfloat));
}

void
GLModel::drawElementApertures(const Element *el)
{
  if (el->hasProperty("optical")) {
    glPushAttrib(GL_LINE_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glColor4fv(m_apertureColor);

    glLineWidth(2);
    const RZ::OpticalElement *optEl = static_cast<const RZ::OpticalElement *>(el);
    auto &surfaces = optEl->opticalSurfaces();
    
    for (auto &surf : surfaces) {
      RZ::GenericAperture *ap = surf->processor->aperture();

      if (ap != nullptr) {
        pushReferenceFrameMatrix(surf->frame);

        ap->renderOpenGL();
        glPopMatrix();
      }
    }
    glPopAttrib();
  }
}

GLfloat *
GLModel::refMatrix()
{
  return m_refMatrix;
}

void
GLModel::setOrientationAndCenter(RZ::Matrix3 const &R, RZ::Vec3 const &O)
{
   GLdouble viewMatrix[16] = {
    R.rows[0].coords[0], R.rows[1].coords[0], R.rows[2].coords[0], 0,
    R.rows[0].coords[1], R.rows[1].coords[1], R.rows[2].coords[1], 0,
    R.rows[0].coords[2], R.rows[1].coords[2], R.rows[2].coords[2], 0,
            O.coords[0],         O.coords[1],         O.coords[2], 1
   };

  glMultMatrixd(viewMatrix);
}

void
GLModel::pushReferenceFrameMatrix(const RZ::ReferenceFrame *frame)
{
  auto R     = frame->getOrientation();
  auto O     = frame->getCenter();

  glPushMatrix();
  glLoadMatrixf(m_refMatrix);
  setOrientationAndCenter(R, O);
}

void
GLModel::pushElementMatrix(Element *element)
{
  pushReferenceFrameMatrix(element->parentFrame());
}

void
GLModel::popElementMatrix()
{
  glPopMatrix();
}
