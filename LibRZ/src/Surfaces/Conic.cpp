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
#include <Logger.h>

using namespace RZ;

ConicSurface::ConicSurface(Real R, Real RCurv, Real K)
{
  m_edges.resize(2);

  setRadius(R);
  setCurvatureRadius(RCurv);
  setConicConstant(K);
  recalcGL();
}

Real
ConicSurface::z(Real r) const
{
  Real sigma = m_convex ? 1 : -1;

  return m_parabola 
    ? -sigma * (.5 / m_rCurv * r * r - m_depth)
    : -sigma * ((m_rCurv - sqrt(m_rCurv2 - (m_K + 1) * r * r)) / (m_K + 1) - m_depth);
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
  m_dirty = true;
}

void
ConicSurface::setRadius(Real R)
{
  m_radius  = R;
  m_radius2 = R * R;

  recalcDistribution();
  m_dirty = true;
}

void
ConicSurface::setCurvatureRadius(Real R)
{
  m_rCurv  = R;
  m_rCurv2 = R * R;

  recalcDistribution();
  m_dirty = true;
}

void
ConicSurface::setConicConstant(Real K)
{
  m_K        = K;
  m_parabola = isZero(K + 1);

  recalcDistribution();
  m_dirty = true;
}

void
ConicSurface::setHoleRadius(Real Rhole)
{
  m_rHole  = Rhole;
  m_rHole2 = Rhole * Rhole;

  recalcDistribution();
  m_dirty = true;
}

void
ConicSurface::setConvex(bool convex)
{
  m_convex = convex;

  recalcDistribution();
  m_dirty = true;
}

std::vector<std::vector<Real>> const &
ConicSurface::edges() const
{
  if (m_dirty)
    const_cast<ConicSurface *>(this)->recalcGL();
  
  return m_edges;
}

void
ConicSurface::generateConicSectionVertices(
      std::vector<GLfloat> &dest,
      Real r0,
      Real rn,
      Real x0, Real y0,
      Real ux, Real uy,
      Real sign,
      unsigned int segments)
{
  Real dh = (rn - r0) / segments;

  Real r = r0;
  Real x, y, z, rho2;
  Real sigma = m_convex ? 1 : -1;

  if (m_parabola) {
    auto inv2R = .5 / m_rCurv;

    for (unsigned i = 0; i < segments + 1; ++i) {
      x = ux * r + x0;
      y = ux * r + y0;
      rho2 = x * x + y * y;
      z = -sigma * (inv2R * rho2 - m_depth);

      dest.push_back(x);
      dest.push_back(y);
      dest.push_back(z);

      r += dh;
    }
  } else {
    auto K1    = m_K + 1;
    auto invK1 = 1 / K1;

    if (K1 < 0)
      sign = 1;

    for (unsigned i = 0; i < segments + 1; ++i) {
      x = ux * r + x0;
      y = uy * r + y0;
      rho2 = x * x + y * y;
      z = -sigma * (invK1 * (m_rCurv - sign * sqrt(m_rCurv2 - K1 * rho2)) - m_depth);

      dest.push_back(x);
      dest.push_back(y);
      dest.push_back(z);

      r += dh;
    }
  }
}

template<class T> void
ConicSurface::generateConicCircle(
      T &dest,
      Real r,
      Real x0, Real y0,
      Real sign,
      unsigned int segments)
{
  Real x, y, z;
  Real rho2;
  Real theta = 0;
  Real dTheta = 2 * M_PI / segments;
  Real sigma = m_convex ? 1 : -1;

  if (m_parabola) {
    auto inv2R = .5 / m_rCurv;
    
    for (unsigned i = 0; i < segments; ++i) {
      x = r * cos(theta) + x0;
      y = r * sin(theta) + y0;
      
      rho2 = x * x + y * y;
      z = -sigma * (inv2R * rho2 - m_depth);

      dest.push_back(x);
      dest.push_back(y);
      dest.push_back(z);

      theta += dTheta;
    }
  } else {
    auto K1 = m_K + 1;
    auto invK1 = 1 / K1;

    for (unsigned i = 0; i < segments; ++i) {
      x = r * cos(theta) + x0;
      y = r * sin(theta) + y0;
      
      rho2 = x * x + y * y;
      z = -sigma * (invK1 * (m_rCurv - sign * sqrt(m_rCurv2 - K1 * rho2)) - m_depth);

      dest.push_back(x);
      dest.push_back(y);
      dest.push_back(z);

      theta += dTheta;
    }
  }
}

//
// For closed conics (K > -1) we find the equator where the curve becomes
// tanget to the Z axis. This is: given the general conic equation
//
// r^2 - 2Rz + (K + 1)z^2 = 0 => r = ± sqrt(2Rz - (K + 1)z^2)
//
// We find the equator where:
//
// dr/dz = 0
//
// If we take the square of r, the result is something that gets stretched
// along the r direction. The equator of this new curve is still at the same
// Z as the original curve:
//
// r^2 = 2Rz -(K + 1)z^2
// dr^2/dz = 0 in the equator
//
// And therefore:
// 
// dr^2/dz = 2R - 2(K + 1)z = 0 => z = R/(K + 1) = z_eq
// r(z_eq) = sqrt(2R^2/(K+1) - (K + 1)R^2/(K+1)^2)
//         = sqrt(2R^2/(K+1) - R^2/(K+1))
//         = sqrt(R^2/(K+1))
//

#define DEFAULT_SEG_SCALE 100
void
ConicSurface::recalcSelectionGL()
{
  Real segScale = DEFAULT_SEG_SCALE;
  Real rEq = DEFAULT_SEG_SCALE * m_radius;
  bool closed = false;

  if (m_K > -1) {
    Real betterREq = fabs(m_rCurv) / sqrt(m_K + 1);
    if (betterREq < DEFAULT_SEG_SCALE * m_radius) {
      rEq      = betterREq * .999999;
      segScale = rEq / m_radius;
      closed   = true;
    }
  }

  m_selectedAxes.clear();
  m_selectedAxesClosed.clear();
  m_selectedEquator.clear();

  generateConicCircle(m_selectedEquator, rEq);

  auto n = fmin(
    static_cast<unsigned>(DEFAULT_SEG_SCALE * GENERIC_APERTURE_NUM_SEGMENTS),
    static_cast<unsigned>(segScale * GENERIC_APERTURE_NUM_SEGMENTS));

  generateConicSectionVertices(m_selectedAxes, 0, rEq, 0, 0, +m_ux, +m_uy, +1, n);
  generateConicSectionVertices(m_selectedAxes, 0, rEq, 0, 0, -m_ux, -m_uy, +1, n);
  generateConicSectionVertices(m_selectedAxes, 0, rEq, 0, 0, -m_uy, +m_ux, +1, n);
  generateConicSectionVertices(m_selectedAxes, 0, rEq, 0, 0, +m_uy, -m_ux, +1, n);

  if (closed) {
    generateConicSectionVertices(m_selectedAxesClosed, 0, rEq, 0, 0, +m_ux, +m_uy, -1, n);
    generateConicSectionVertices(m_selectedAxesClosed, 0, rEq, 0, 0, -m_ux, -m_uy, -1, n);
    generateConicSectionVertices(m_selectedAxesClosed, 0, rEq, 0, 0, -m_uy, +m_ux, -1, n);
    generateConicSectionVertices(m_selectedAxesClosed, 0, rEq, 0, 0, +m_uy, -m_ux, -1, n);
  }
}

void
ConicSurface::recalcGL()
{ 
  m_vertices.clear();
  m_holeVertices.clear();
  m_edges[0].clear();
  m_edges[1].clear();
  m_axes.clear();

  generateConicCircle(m_vertices, m_radius, m_x0, m_y0);
  generateConicCircle(m_edges[0], m_radius, m_x0, m_y0);

  if (m_rHole > 0) {
    generateConicCircle(m_holeVertices, m_rHole, m_x0, m_y0);
    generateConicCircle(m_edges[1], m_rHole, m_x0, m_y0);
  }

  generateConicSectionVertices(m_axes, m_rHole, m_radius, m_x0, m_y0, +m_ux, +m_uy);
  generateConicSectionVertices(m_axes, m_rHole, m_radius, m_x0, m_y0, -m_ux, -m_uy);
  generateConicSectionVertices(m_axes, m_rHole, m_radius, m_x0, m_y0, -m_uy, +m_ux);
  generateConicSectionVertices(m_axes, m_rHole, m_radius, m_x0, m_y0, +m_uy, -m_ux);

  //recalcSelectionGL();

  m_dirty = false;
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

  // Note: The equation is A^2t + Bt + C = 0. This implies certain numerical
  // precision issue here when 4AC << B^2. For now, this test seems sufficient
  // but this requires a more careful approach.

  if (isZero(A)) {
    // Case Bt + C = 0. There is only one solution in this case.
    deltaT = -C / B;
  } else if (Delta >= 0) {
    Real sPart = .5 * sqrt(Delta) / A;
    Real fPart = -.5 * B/A;

    Real dt1 = fPart + sPart;
    Real dt2 = fPart - sPart;
    
    if (dt1 * dt2 < 0) // One of the two is negative. Pick the positive.
      deltaT = fmax(dt1, dt2);
    else
      deltaT = fmin(dt1, dt2); // Both are of the same sign. Pick the smallest.
  } else {
    return false;
  }

  if (deltaT < 0)
    return false;
  
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
  if (m_dirty)
    recalcGL();
  
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

void
ConicSurface::renderOpenGLExtra()
{
  if (m_dirty)
    recalcGL();
  
  glEnableClientState(GL_VERTEX_ARRAY);
    glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT);

    glColor4f(1, 1, 0, 1);
    glLineWidth(1.);
    glLineStipple(1, 0xF0F0);
    glEnable(GL_LINE_STIPPLE);

    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_selectedEquator.data());
    glDrawArrays(GL_LINE_LOOP, 0, m_selectedEquator.size() / 3);

    auto N = m_selectedAxes.size() / (4 * 3);
  
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_selectedAxes.data());
    glDrawArrays(GL_LINE_STRIP, 0, N);

    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_selectedAxes.data() + 3 * N);
    glDrawArrays(GL_LINE_STRIP, 0, N);

    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_selectedAxes.data() + 6 * N);
    glDrawArrays(GL_LINE_STRIP, 0, N);
    
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), m_selectedAxes.data() + 9 * N);
    glDrawArrays(GL_LINE_STRIP, 0, N);

    glPopAttrib();
  glDisableClientState(GL_VERTEX_ARRAY);
}

