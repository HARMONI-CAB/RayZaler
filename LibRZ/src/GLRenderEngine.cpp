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
#include <GLModel.h>

using namespace RZ;

GLHelperGrid::GLHelperGrid()
{
  m_glGridText.setFace("gridfont-semibold");

  setColor(m_color);
  setGridStep(1e-3);
  setGridDivs(100);
}

void
GLHelperGrid::display()
{
  m_xyCoarseGrid.display();
  m_xyMediumGrid.display();
  m_xyFineGrid.display();

  glPushMatrix();
    glTranslatef(-m_xyFineGrid.width() / 2, +m_xyFineGrid.height() / 2 + m_xyFineGrid.step(), 0);
    m_glGridText.display();
  glPopMatrix();
}

void
GLHelperGrid::setColor(GLfloat r, GLfloat g, GLfloat b)
{
  m_xyCoarseGrid.setGridColor(r, g, b, 1);
  m_xyCoarseGrid.setHighlightColor(1, 1, 0, 1);
  m_xyMediumGrid.setGridColor(r, g, b, .75);
  m_xyFineGrid.setGridColor(r, g, b, .5);
  m_glGridText.setColor(r, g, b);
}

void
GLHelperGrid::setColor(const GLfloat *color)
{
  setColor(color[0], color[1], color[2]);
}

void
GLHelperGrid::setGridText(std::string const &text)
{
  m_glGridText.setText(text);
}

void
GLHelperGrid::setGridStep(Real step)
{
  m_step = step;
  m_xyCoarseGrid.setStep(step * 10);
  m_xyMediumGrid.setStep(step * 5);
  m_xyFineGrid.setStep(step);
  m_glGridText.setScale(step * 1e-1);
}

void
GLHelperGrid::setGridDivs(unsigned int num)
{
  if (num != m_divs) {
    m_divs = num;

    m_xyCoarseGrid.setStepsX(num / 10);
    m_xyCoarseGrid.setStepsY(num / 10);

    m_xyMediumGrid.setStepsX(num / 5);
    m_xyMediumGrid.setStepsY(num / 5);

    m_xyFineGrid.setStepsX(num);
    m_xyFineGrid.setStepsY(num);
  }
}

void
GLHelperGrid::highlight(Real x, Real y)
{
  m_xyCoarseGrid.highlight(x, y);
}

////////////////////////////// GLCurrentView ///////////////////////////////////
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

void
GLRenderEngine::drawGrids()
{
  auto zoom = m_view.zoomLevel;

  for (auto &grid : m_grids) {
    m_model->pushReferenceFrameMatrix(grid.frame);
    grid.grid.display();
    glScalef(1. / zoom, 1. / zoom, 1. / zoom);
    m_glAxes.display();
    glPopMatrix();
  }
}

GLFrameGrid *
GLRenderEngine::addGrid(std::string const &name, ReferenceFrame const *frame)
{
  GLFrameGrid newGrid;


  newGrid.frame = frame;
  
  m_grids.push_back(newGrid);

  auto &last = m_grids.back();
  last.grid.setGridText(name);
  last.grid.setColor(0, 0, 0);

  return &last;
}

GLModel *
GLRenderEngine::model() const
{
  return m_model;
}

void
GLRenderEngine::setOrientationAndCenter(RZ::Matrix3 const &R, RZ::Vec3 const &O)
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
GLRenderEngine::drawCornerAxes()
{
  GLfloat axisHeight = m_glAxes.height();
  GLfloat aspect = m_view.width / m_view.height;
  
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
    glLoadIdentity();
    glOrtho(-2, 2, -2 / aspect, 2 / aspect, -20, 20);

    glMatrixMode(GL_MODELVIEW);
      glPushMatrix();

      glTranslatef(2 - 1.5 * axisHeight, -2 / aspect + 1.5 * axisHeight, 0.);

      m_view.configureOrientation(false);
      
      m_glAxes.display();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode (GL_MODELVIEW);
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
