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


#include <Surfaces/Conic.h>

using namespace RZ;

ConicSurface::ConicSurface(Real R, Real RCurv, Real K)
{
  m_edges.resize(2);

  setRadius(R);
  setCurvatureRadius(RCurv);
  setConicConstant(K);
  recalcGL();
}

void
ConicSurface::recalcDistribution()
{
  Real x2, y2, invnorm;
  /* TODO */

  if (m_parabola)
    m_depth = .5 * m_radius2 / m_rCurv;
  else
    m_depth = (m_rCurv - sqrt(m_rCurv2 - (m_K + 1) * m_radius2)) / (m_K + 1);
  
  x2 = m_x0 * m_x0;
  y2 = m_y0 * m_y0;

  if (isZero(x2 + y2)) {
    m_ux = 1;
    m_uy = 0;
  } else {
    invnorm = 1 / sqrt(x2 + y2);
    m_ux = invnorm * m_x0;
    m_uy = invnorm * m_y0;
  }
}

void
ConicSurface::setCenterOffset(Real x, Real y)
{
  m_x0 = x;
  m_y0 = y;

  recalcDistribution();
  recalcGL();
}

void
ConicSurface::setRadius(Real R)
{
  m_radius  = R;
  m_radius2 = R * R;

  recalcDistribution();
  recalcGL();
}

void
ConicSurface::setCurvatureRadius(Real R)
{
  m_rCurv  = R;
  m_rCurv2 = R * R;

  recalcDistribution();
  recalcGL();
}

void
ConicSurface::setConicConstant(Real K)
{
  m_K        = K;
  m_parabola = isZero(K + 1);

  recalcDistribution();
  recalcGL();
}

void
ConicSurface::setHoleRadius(Real Rhole)
{
  m_rHole  = Rhole;
  m_rHole2 = Rhole * Rhole;

  recalcDistribution();
  recalcGL();
}

void
ConicSurface::setConvex(bool convex)
{
  m_convex = convex;

  recalcDistribution();
  recalcGL();
}

std::vector<std::vector<Real>> const &
ConicSurface::edges() const
{
  return m_edges;
}

void
ConicSurface::recalcGLConic()
{
  GLfloat x, y, z, r, dh;
  Real theta = 0;
  Real dTheta = 2 * M_PI / GENERIC_APERTURE_NUM_SEGMENTS;
  Real rho2;
  Real sigma = m_convex ? 1 : -1;

  m_vertices.clear();
  m_holeVertices.clear();

  auto K1 = m_K + 1;
  auto invK1 = 1 / K1;

  m_edges[0].clear();
  m_edges[1].clear();

  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS; ++i) {
    x = m_radius * cos(theta) + m_x0;
    y = m_radius * sin(theta) + m_y0;
    
    rho2 = x * x + y * y;
    z = -sigma * (invK1 * (m_rCurv - sqrt(m_rCurv2 - K1 * rho2)) - m_depth);

    m_vertices.push_back(x);
    m_vertices.push_back(y);
    m_vertices.push_back(z);

    m_edges[0].push_back(x);
    m_edges[0].push_back(y);
    m_edges[0].push_back(z);

    if (m_rHole > 0) {
      x = m_rHole * cos(theta) + m_x0;
      y = m_rHole * sin(theta) + m_y0;
      
      rho2 = x * x + y * y;
      z = -sigma * (invK1 * (m_rCurv - sqrt(m_rCurv2 - K1 * rho2)) - m_depth);

      m_holeVertices.push_back(x);
      m_holeVertices.push_back(y);
      m_holeVertices.push_back(z);

      m_edges[1].push_back(x);
      m_edges[1].push_back(y);
      m_edges[1].push_back(z);
    }

    theta += dTheta;
  }
  

  // Draw the aperture ellipse
 
  m_axes.clear();

  dh = (m_radius - m_rHole) / GENERIC_APERTURE_NUM_SEGMENTS;

  r = m_rHole;
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS + 1; ++i) {
    x = m_ux * r + m_x0;
    y = m_uy * r + m_y0;
    rho2 = x * x + y * y;
    z = -sigma * (invK1 * (m_rCurv - sqrt(m_rCurv2 - K1 * rho2)) - m_depth);

    m_axes.push_back(x);
    m_axes.push_back(y);
    m_axes.push_back(z);

    r += dh;
  }

  r = -m_rHole;
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS + 1; ++i) {
    x = m_ux * r + m_x0;
    y = m_uy * r + m_y0;
    rho2 = x * x + y * y;
    z = -sigma * (invK1 * (m_rCurv - sqrt(m_rCurv2 - K1 * rho2)) - m_depth);

    m_axes.push_back(x);
    m_axes.push_back(y);
    m_axes.push_back(z);

    r -= dh;
  }

  r = m_rHole;
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS + 1; ++i) {
    x = -m_uy * r + m_x0;
    y = m_ux * r + m_y0;
    rho2 = x * x + y * y;
    z = -sigma * (invK1 * (m_rCurv - sqrt(m_rCurv2 - K1 * rho2)) - m_depth);

    m_axes.push_back(x);
    m_axes.push_back(y);
    m_axes.push_back(z);
  
    r += dh;
  }

  r = -m_rHole;
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS + 1; ++i) {
    x = -m_uy * r + m_x0;
    y = m_ux * r + m_y0;
    rho2 = x * x + y * y;
    z = -sigma * (invK1 * (m_rCurv - sqrt(m_rCurv2 - K1 * rho2)) - m_depth);

    m_axes.push_back(x);
    m_axes.push_back(y);
    m_axes.push_back(z);
  
    r -= dh;
  }
}

void
ConicSurface::recalcGLParabolic()
{
  GLfloat x, y, z, r, dh;
  Real theta = 0;
  Real dTheta = 2 * M_PI / GENERIC_APERTURE_NUM_SEGMENTS;
  Real rho2;
  Real sigma = m_convex ? 1 : -1;
  bool haveHole = m_rHole > 0;

  m_vertices.clear();
  m_holeVertices.clear();

  auto inv2R = .5 / m_rCurv;

  m_edges[0].clear();
  m_edges[1].clear();
  
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS; ++i) {
    x = m_radius * cos(theta) + m_x0;
    y = m_radius * sin(theta) + m_y0;
    
    rho2 = x * x + y * y;
    z = -sigma * (inv2R * rho2 - m_depth);

    m_vertices.push_back(x);
    m_vertices.push_back(y);
    m_vertices.push_back(z);

    m_edges[0].push_back(x);
    m_edges[0].push_back(y);
    m_edges[0].push_back(z);

    if (haveHole > 0) {
      x = m_rHole * cos(theta) + m_x0;
      y = m_rHole * sin(theta) + m_y0;
      
      rho2 = x * x + y * y;
      z = -sigma * (inv2R * rho2 - m_depth);

      m_holeVertices.push_back(x);
      m_holeVertices.push_back(y);
      m_holeVertices.push_back(z);

      m_edges[1].push_back(x);
      m_edges[1].push_back(y);
      m_edges[1].push_back(z);
    }

    theta += dTheta;
  }

  m_axes.clear();

  dh = (m_radius - m_rHole) / GENERIC_APERTURE_NUM_SEGMENTS;

  r = m_rHole;
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS + 1; ++i) {
    x = m_ux * r + m_x0;
    y = m_uy * r + m_y0;
    rho2 = x * x + y * y;
    z = -sigma * (inv2R * rho2 - m_depth);

    m_axes.push_back(x);
    m_axes.push_back(y);
    m_axes.push_back(z);

    r += dh;
  }

  r = -m_rHole;
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS + 1; ++i) {
    x = m_ux * r + m_x0;
    y = m_uy * r + m_y0;
    rho2 = x * x + y * y;
    z = -sigma * (inv2R * rho2 - m_depth);

    m_axes.push_back(x);
    m_axes.push_back(y);
    m_axes.push_back(z);

    r -= dh;
  }

  r = m_rHole;
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS + 1; ++i) {
    x = -m_uy * r + m_x0;
    y = m_ux * r + m_y0;
    rho2 = x * x + y * y;
    z = -sigma * (inv2R * rho2 - m_depth);

    m_axes.push_back(x);
    m_axes.push_back(y);
    m_axes.push_back(z);
  
    r += dh;
  }

  r = -m_rHole;
  for (unsigned i = 0; i < GENERIC_APERTURE_NUM_SEGMENTS + 1; ++i) {
    x = -m_uy * r + m_x0;
    y = m_ux * r + m_y0;
    rho2 = x * x + y * y;
    z = -sigma * (inv2R * rho2 - m_depth);

    m_axes.push_back(x);
    m_axes.push_back(y);
    m_axes.push_back(z);
  
    r -= dh;
  }
}

void
ConicSurface::recalcGL()
{ 
  if (m_parabola)
    recalcGLParabolic();
  else
    recalcGLConic();
}

bool
ConicSurface::intercept(
  Vec3 &intercept,
  Vec3 &normal,
  Real &deltaT,
  Vec3 const &Ot,
  Vec3 const &ut) const
{
  Real x0 = Ot.x;
  Real y0 = Ot.y;
  Real z0 = Ot.z;

  Real a = ut.x;
  Real b = ut.y;
  Real c = ut.z;

  // See GeneralConic.ipynb for details
  Real K1    = m_K + 1;        // K + 1
  Real D     = m_depth;
  Real DK1   = D * K1;         // D(K + 1) = DK + D
  Real RDKD  = m_rCurv - DK1;  // R - D(K + 1) = R - DK - D
  Real sigma = m_convex ? 1 : -1;
  Real rho2;

  Real A     = a * a + b * b + K1 * c * c;
  Real B     = 2 * (a * x0 + b * y0 + K1 * c * z0 + sigma * c * RDKD);
  Real C     = x0 * x0 + y0 * y0 + K1 * z0 * z0 + 2 * sigma * RDKD * z0 - 2 * D * m_rCurv + DK1 * D;

  Real Delta = B * B - 4 * A * C;
  Real t;

  if (fabs(A) <= std::numeric_limits<Real>::epsilon()) {
    // Case Bt + C = 0. There is only one solution in this case.
    deltaT = -C / B;
  } else if (Delta >= 0) {
    deltaT = .5 * (-B - sigma * sqrt(Delta)) / A;
  } else {
    return false;
  }

  // Vignetting test is performed only after computing the intersection with
  // the paraboloid.
  Real x, y;
  intercept = Ot + deltaT * ut;
  x = intercept.x - m_x0;
  y = intercept.y - m_y0;

  rho2 = x * x + y * y;

  normal = Vec3(
            sigma * intercept.x,
            sigma * intercept.y,
            sigma * K1 * intercept.z + RDKD).normalized();


  if (rho2 >= m_radius2 || rho2 < m_rHole2)
    return complementary();

  return !complementary();
}

void
ConicSurface::generatePoints(
    const ReferenceFrame *frame,
    Real *pointArr,
    Real *normalArr,
    unsigned int N)
{
  throw std::runtime_error("Sampling of general conic surfaces not yet implemented");
}

Real
ConicSurface::area() const
{
  throw std::runtime_error("Surface area of a general conic not yet implemented");
}

std::string
ConicSurface::name() const
{
  return "Conic(" + std::to_string(m_K) + ")";
}

void
ConicSurface::renderOpenGL()
{
  auto N = m_axes.size() / (4 * 3);

  glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_vertices.data());
    glDrawArrays(GL_LINE_LOOP, 0, m_vertices.size() / 3);

    if (!m_holeVertices.empty()) {
      glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_holeVertices.data());
      glDrawArrays(GL_LINE_LOOP, 0, m_holeVertices.size() / 3);
    }
  
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_axes.data());
    glDrawArrays(GL_LINE_STRIP, 0, N);

    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_axes.data() + 3 * N);
    glDrawArrays(GL_LINE_STRIP, 0, N);

    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_axes.data() + 6 * N);
    glDrawArrays(GL_LINE_STRIP, 0, N);
    
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_axes.data() + 9 * N);
    glDrawArrays(GL_LINE_STRIP, 0, N);

  glDisableClientState(GL_VERTEX_ARRAY);
}
