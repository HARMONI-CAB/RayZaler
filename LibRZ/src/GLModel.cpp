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
