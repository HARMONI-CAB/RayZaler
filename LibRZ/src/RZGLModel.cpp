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
#include <Elements/RayBeamElement.h>

using namespace RZ;

void
RZGLModel::displayModel(OMModel *model)
{
  for (auto p : model->elementList()) {
    if (m_showElements) {
      pushElementMatrix(p);
      
      if (p == m_bbElement)
        p->renderBoundingBoxOpenGL();
      
      p->renderOpenGL();
      popElementMatrix();
    }

    if (m_showApertures)
      drawElementApertures(p);
    
    if (p->nestedModel() != nullptr)
      displayModel(p->nestedModel());
  }

  if (model == m_model) {
    auto beam = model->beam();
    pushElementMatrix(beam);
    beam->renderOpenGL();
    popElementMatrix();
  }
}

void
RZGLModel::display()
{
  tick();
  
  glGetFloatv(GL_MODELVIEW_MATRIX, refMatrix());

  updateRefMatrix();

  displayModel(m_model);
}


void
RZGLModel::pushOptoMechanicalModel(OMModel *om)
{
  m_model = om;
  m_model->recalculate();
}

void
RZGLModel::setShowApertures(bool show)
{
  m_showApertures = show;
}

void
RZGLModel::setShowElements(bool show)
{
  m_showElements = show;
}

void
RZGLModel::setHighlightedBoundingBox(Element *element)
{
  m_bbElement = element;

  if (element != nullptr)
    element->calcBoundingBoxOpenGL();
}
