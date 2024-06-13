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

#include <GLRenderEngine.h>
#include <GLHelpers.h>

using namespace RZ;

void
GLCurrentView::zoom(Real delta)
{
  zoomLevel *= delta;
}

void
GLCurrentView::incAzEl(Real deltaAz, Real deltaEl)
{
  rotation.rotate(RZ::Vec3::eY(), RZ::deg2rad(deltaAz));
  rotation.rotate(RZ::Vec3::eX(), RZ::deg2rad(deltaEl));
}

void
GLCurrentView::roll(Real delta)
{
  rotation.rotate(RZ::Vec3::eZ(), RZ::deg2rad(delta));
}

void
GLCurrentView::move(Real deltaX, Real deltaY)
{
  center[0] -= deltaX;
  center[1] -= deltaY;
}

void
GLCurrentView::setZoom(Real zoom)
{
  zoomLevel = zoom;
}

void
GLCurrentView::setCenter(Real x0, Real y0)
{
  center[0] = x0;
  center[1] = y0;
}

void
GLCurrentView::setScreenGeom(Real width, Real height)
{
  this->width  = width;
  this->height = height;
}

void
GLCurrentView::setRotation(Real angle, Real x, Real y, Real z)
{
  rotation.setRotation(Vec3(x, y, z).normalized(), RZ::deg2rad(angle));
}

void
GLCurrentView::setRotation(RZ::Vec3 const &vec, Real angle)
{
  rotation.setRotation(vec.normalized(), RZ::deg2rad(angle));
}

void
GLCurrentView::setRotation(RZ::Matrix3 const &matrix)
{
  rotation.setRotation(matrix);
}

void
GLCurrentView::rotateRelative(RZ::Vec3 const &vec, Real angle)
{
  rotation.rotateRelative(vec.normalized(), RZ::deg2rad(angle));
}

void
GLCurrentView::configureViewPort(unsigned int width, unsigned int height) const
{
  GLfloat aspect;

  // Phase 1: Set viewport
  glViewport(0, 0, width, height);

  // Phase 2: Configure projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glScalef(zoomLevel, zoomLevel, zoomLevel);
  glTranslatef(
    +2 * center[0] / (zoomLevel * width),
    -2 * center[1] / (zoomLevel * height),
    0);
    
  aspect = static_cast<GLfloat>(width) / static_cast<GLfloat>(height);
  glOrtho(-2, 2, -2 / aspect, 2 / aspect, -20 * zoomLevel, 20 * zoomLevel);

  // Phase 3: Configure normals
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_AUTO_NORMAL);
  glEnable(GL_NORMALIZE);
}

void
GLCurrentView::configureOrientation(bool translate) const
{
  auto k = rotation.k();
  auto theta = RZ::rad2deg(rotation.theta());

  if (translate)
    glTranslatef(0, 0, -10.);
  
  glRotatef(theta, k.x, k.y, k.z);

  glRotatef(-90, 1, 0, 0);
  glRotatef(-90, 0, 0, 1);
}

void
GLCurrentView::screenToWorld(Real &wX, Real &wY, Real sX, Real sY)
{
  // IMPORTANT: Both horizontal and vertical zoom levels are given by the width

  wX = +4 * (sX - center[0] - width  * .5) / (zoomLevel * width) - 2;
  wY = -4 * (sY - center[1] - height * .5) / (zoomLevel * width) + 2;
}

void
GLCurrentView::worldToScreen(Real &sX, Real &sY, Real wX, Real wY)
{
  sX = +.25 * (wX + 2) * (zoomLevel * width) + center[0] + width  * .5;
  sY = -.25 * (wY - 2) * (zoomLevel * width) + center[1] + height  * .5;
}

///////////////////////////// GLRenderEngine ///////////////////////////////////
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

GLCurrentView *
GLRenderEngine::view()
{
  return &m_view;
}

void
GLRenderEngine::zoom(Real delta)
{
  m_view.zoom(delta);
}

void
GLRenderEngine::incAzEl(Real deltaAz, Real deltaEl)
{
  m_view.incAzEl(deltaAz, deltaEl);
}

void
GLRenderEngine::roll(Real delta)
{
  m_view.roll(delta);
}

void
GLRenderEngine::move(Real deltaX, Real deltaY)
{
  m_view.move(deltaX, deltaY);
}

void
GLRenderEngine::setZoom(Real zoom)
{
  m_view.setZoom(zoom);
}

void
GLRenderEngine::setCenter(Real x, Real y)
{
  m_view.setCenter(x, y);
}

void
GLRenderEngine::setRotation(Real angle, Real x, Real y, Real z)
{
  m_view.setRotation(angle, x, y, z);
}

void
GLRenderEngine::setView(GLCurrentView const *view)
{
  int savedWidth, savedHeight;

  savedWidth  = m_view.width;
  savedHeight = m_view.height;

  m_view = *view;

  m_view.setScreenGeom(savedWidth, savedHeight);
}
