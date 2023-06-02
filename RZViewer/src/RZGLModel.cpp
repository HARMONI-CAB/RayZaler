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
RZGLModel::display()
{
  tick();
  
  glGetFloatv(GL_MODELVIEW_MATRIX, m_refMatrix);

  for (auto p : m_elements) {
    pushElementMatrix(p);
    p->renderOpenGL();
    popElementMatrix();
  }
}

void
RZGLModel::pushElement(Element *element)
{
  m_elements.push_back(element);
}

void
RZGLModel::pushOptoMechanicalModel(OMModel *om)
{
  om->recalculate();

  for (auto p : om->elementList())
    if (p != om->beam())
      pushElement(p);

  pushElement(om->beam());
}
