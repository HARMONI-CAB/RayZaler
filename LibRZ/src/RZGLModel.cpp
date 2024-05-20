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

#include <RZGLModel.h>
#include <Element.h>
#include <OMModel.h>

using namespace RZ;

void
RZGLModel::pushElementMatrix(Element *element)
{
  auto frame = element->parentFrame();
  auto R     = frame->getOrientation();
  auto O     = frame->getCenter();

  GLdouble viewMatrix[16] = {
    R.rows[0].coords[0], R.rows[1].coords[0], R.rows[2].coords[0], O.coords[0],
    R.rows[0].coords[1], R.rows[1].coords[1], R.rows[2].coords[1], O.coords[1],
    R.rows[0].coords[2], R.rows[1].coords[2], R.rows[2].coords[2], O.coords[2],
                      0,                   0,                   0,           1};
  
  glPushMatrix();
  glLoadMatrixf(m_refMatrix);
  glMultTransposeMatrixd(viewMatrix);
}

void
RZGLModel::popElementMatrix()
{
  glPopMatrix();
}

void
RZGLModel::displayModel(OMModel *model)
{
  for (auto p : model->elementList()) {
    pushElementMatrix(p);
    p->renderOpenGL();
    popElementMatrix();

    if (p->nestedModel() != nullptr)
      displayModel(p->nestedModel());
  }
}

void
RZGLModel::display()
{
  tick();
  
  glGetFloatv(GL_MODELVIEW_MATRIX, m_refMatrix);

  displayModel(m_model);
}


void
RZGLModel::pushOptoMechanicalModel(OMModel *om)
{
  m_model = om;
  m_model->recalculate();
}
