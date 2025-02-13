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

#include <MediumBoundaries/LensletArray.h>
#include <ReferenceFrame.h>
#include <Surfaces/Array.h>
#include <Surfaces/Conic.h>

using namespace RZ;

// In the lenslet array we basically have an array of partially overlapped
// convex surfaces.

LensletArrayProcessor::LensletArrayProcessor()
{
  setSurfaceShape(
    new SurfaceArray(
      new ConicSurface(1e-2, m_rCurv, m_K)));

  recalculateDimensions();
}

void
LensletArrayProcessor::recalculateDimensions()
{
  if (m_dirty) {
    auto *array   = surfaceShape<SurfaceArray>();
    auto *lenslet = array->subAperture<ConicSurface>();

    Real lensletWidth  = array->subApertureWidth();
    Real lensletHeight = array->subApertureHeight();
    Real radius = 
      .5 * sqrt(
        lensletWidth * lensletWidth + lensletHeight * lensletHeight);

    // Calculate curvature center
    m_center = sqrt(m_rCurv * m_rCurv - radius * radius);
    m_IOratio = m_muIn / m_muOut;

    lenslet->setRadius(radius);
    lenslet->setCurvatureRadius(m_rCurv);
    lenslet->setConvex(m_convex);
    lenslet->setConicConstant(m_K);

    // Cache lenslet radius
    m_lensletRadius = radius;
    
    m_dirty = false;
  }
}

void
LensletArrayProcessor::setCurvatureRadius(Real rCurv)
{
  m_rCurv = rCurv;
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setRefractiveIndex(Real in, Real out)
{
  m_muIn  = in;
  m_muOut = out;
  m_dirty = true;
  recalculateDimensions();
}
      
std::string
LensletArrayProcessor::name() const
{
  return "LensletArrayProcessor";
}

void
LensletArrayProcessor::setWidth(Real width)
{
  surfaceShape<SurfaceArray>()->setWidth(width);
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setHeight(Real height)
{
  surfaceShape<SurfaceArray>()->setHeight(height);
  m_dirty  = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setCols(unsigned cols)
{
  surfaceShape<SurfaceArray>()->setCols(cols);
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setConicConstant(Real K)
{
  m_K = K;
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setConvex(bool convex)
{
  m_convex = convex;
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::setRows(unsigned rows)
{
  surfaceShape<SurfaceArray>()->setRows(rows);
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  Vec3 normal;
  uint64_t i;
  
  for (i = 0; i < count; ++i) {
    Vec3 origin = plane->toRelative(Vec3(beam.origins + 3 * i));
    Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));
    Real dt     = 0;

    if (surfaceShape()->intercept(coord, normal, dt, origin)) {
      beam.lengths[i]       += dt;
      beam.cumOptLengths[i] += beam.n * dt;
      plane->fromRelative(coord).copyToArray(beam.destinations + 3 * i);
      beam.interceptDone(i);
      
      snell(
        Vec3(beam.directions + 3 * i), 
        plane->fromRelativeVec(normal),
        m_IOratio).copyToArray(beam.directions + 3 * i);
    } else {
      // Outside lens
      beam.prune(i);
    }
  }

  beam.n = m_muIn;
}
