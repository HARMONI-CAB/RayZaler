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

#ifndef _GL_MODEL_H
#define _GL_MODEL_H

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "IncrementalRotation.h"
#include "Element.h"

namespace RZ {
  class GLModelEventListener {
    public:
      virtual void tick() = 0;
  };

  class GLModel {
      GLModelEventListener *m_listener = nullptr;
      GLfloat                m_refMatrix[16];
      GLfloat                m_apertureColor[4] = {0, 0, 1, 1};
      unsigned int           m_thickness = 3;

    protected:
      void tick();
      void drawElementApertures(const Element *);
      
    public:
      void setOrientationAndCenter(RZ::Matrix3 const &R, RZ::Vec3 const &O);
      void pushReferenceFrameMatrix(const RZ::ReferenceFrame *frame);
      void pushElementMatrix(Element *);
      void popElementMatrix();

      GLfloat *refMatrix();
      void updateRefMatrix();
      
      void setApertureColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
      void setApertureColor(const GLfloat *rgb);
      void setApertureColor(Vec3 const &);
      void setApertureThickness(unsigned int);

      void setEventListener(GLModelEventListener *listener);
      virtual void configureLighting();
      virtual void display() = 0;
  };
}

#endif // _GL_MODEL_H
