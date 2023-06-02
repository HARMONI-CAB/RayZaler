#include <GLRenderEngine.h>

using namespace RZ;

void
GLRenderEngine::setModel(GLModel *model)
{
  m_model = model;
}

GLModel *
GLRenderEngine::model() const
{
  return m_model;
}
