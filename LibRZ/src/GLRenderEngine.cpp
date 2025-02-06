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
GLHelperGrid::setColor(Vec3 const &vec)
{
  setColor(vec.coords[0], vec.coords[1], vec.coords[2]);
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
GLHelperGrid::setThickness(unsigned thickness)
{
  m_xyFineGrid.setThickness(thickness);
  m_xyMediumGrid.setThickness(thickness);
  m_xyCoarseGrid.setThickness(thickness);
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
GLCurrentView::setScreenRotation(IncrementalRotation const &rot)
{
  screenRotation = rot;
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
  glOrtho(-2, +2, -2 / aspect, +2 / aspect, -20 * zoomLevel, 20 * zoomLevel);

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
GLCurrentView::screenToWorld(Real &wX, Real &wY, Real sX, Real sY) const
{
  // IMPORTANT: Both horizontal and vertical zoom levels are given by the width

  wX = +4 * (sX - center[0] - width  * .5) / (zoomLevel * width);
  wY = -4 * (sY - center[1] - height * .5) / (zoomLevel * width);
}

void
GLCurrentView::worldToScreen(Real &sX, Real &sY, Real wX, Real wY) const
{
  sX = +.25 * wX * (zoomLevel * width) + center[0] + width  * .5;
  sY = -.25 * wY * (zoomLevel * width) + center[1] + height  * .5;
}

//
//     RZ::IncrementalRotation correctedRotation = m_view.rotation;

//correctedRotation.rotateRelative(RZ::Vec3::eX(), -M_PI / 2);
//correctedRotation.rotateRelative(RZ::Vec3::eZ(), -M_PI / 2);
//
bool
GLCurrentView::worldToFrame(
    Vec3 &result,
    ReferenceFrame const &frame,
    Real wX,
    Real wY) const
{
  RZ::Vec3 coords(wX, wY, 0);
  RZ::Vec3 screenNormal, screenCoords;

  IncrementalRotation currRot = rotation;
  currRot.rotate(screenRotation);
  Matrix3 screenMatrix = currRot.matrix();

  //
  // p0: m_selectedFrame.getCenter()
  // sx: correctedRotation.matrix().vx
  // sy: correctedRotation.matrix().vy
  // ns: correctedRotation.matrix().vz
  // z:  m_selectedFrame.z
  //

  auto sx    = screenMatrix.vx();
  auto sy    = screenMatrix.vy();
  auto ns    = screenMatrix.vz();
  auto dx    = wX * sx;
  auto dy    = wY * sy;
  auto ez    = frame.getOrientation().t().vz();

  auto p0    = frame.getCenter();
  auto sproj = (p0 - dx - dy) * ez;
  auto nproj = ns * ez;

  if (!RZ::isZero(nproj)) {
    RZ::Real t  = sproj / nproj;
    auto pt     = dx + dy + t * ns; // Point in the frame
    result = frame.toRelative(pt);
    return true;
  }

  return false;
}

bool
GLCurrentView::screenToFrame(
    Vec3 &result,
    ReferenceFrame const &frame,
    Real sX,
    Real sY) const
{
  Real wX, wY;

  screenToWorld(wX, wY, sX, sY);
  return worldToFrame(result, frame, wX, wY);
}

void
GLCurrentView::frameToWorld(
    Real &wX,
    Real &wY,
    ReferenceFrame const &frame,
    Vec3 const &vec) const
{
  IncrementalRotation currRot = rotation;
  currRot.rotate(screenRotation);
  Matrix3 screenMatrix = currRot.matrix();

  auto pt = frame.fromRelative(vec);
  auto world = screenMatrix * pt;

  wX = world.x;
  wY = world.y;
}

void
GLCurrentView::frameToScreen(
    Real &sX,
    Real &sY,
    ReferenceFrame const &frame,
    Vec3 const &vec) const
{
  Real wX, wY;

  frameToWorld(wX, wY, frame, vec);
  worldToScreen(sX, sY, wX, wY);
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
    glScalef(m_axisZoom / zoom, m_axisZoom / zoom, m_axisZoom / zoom);
    m_glAxes.display();
    glPopMatrix();
  }
}

void
GLRenderEngine::setAxesZoom(Real zoom)
{
  m_axisZoom = zoom;
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
GLCurrentView::zoomToBox(
  ReferenceFrame const &ref,
  Vec3 const &p1,
  Vec3 const &p2)
{
  RZ::Real sX = 0, sY = 0;
  RZ::Vec3 center = .5 * (p1 + p2);

  RZ::Matrix3 rotation =
      RZ::Matrix3::rot(RZ::Vec3::eX(), +.25 * M_PI) *
      RZ::Matrix3::rot(RZ::Vec3::eY(), -.25 * M_PI);

  setRotation(rotation);
  frameToScreen(sX, sY, ref, center);

  // Calculate screen bounding box
  RZ::Vec3 s1, s2;

  for (auto i = 0; i < 8; ++i) {
    RZ::Real cornerX, cornerY;
    bool whichX = (i & 1) != 0;
    bool whichY = (i & 2) != 0;
    bool whichZ = (i & 4) != 0;

    RZ::Vec3 p = RZ::Vec3(
      whichX ? p1.x : p2.x,
      whichY ? p1.y : p2.y,
      whichZ ? p1.z : p2.z);

    frameToScreen(cornerX, cornerY, ref, p);

    RZ::Vec3 s(cornerX, cornerY, 0);

    if (i == 0)
      s1 = s2 = s;
    else
      expandBox(s1, s2, s);
  }

  // Correct zoom
  auto geom = s2 - s1;
  setZoom(fmin(width / (1e-12 + geom.x), height / (1e-12 + geom.y)));

  // Center
  frameToScreen(sX, sY, ref, center);
  setCenter(-(sX - width / 2), -(sY - height / 2));
}

void
GLRenderEngine::zoomToBox(
  ReferenceFrame const &ref,
  Vec3 const &p1,
  Vec3 const &p2)
{
  m_view.zoomToBox(ref, p1, p2);
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
