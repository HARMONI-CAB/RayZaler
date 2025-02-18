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

#include <EMInterfaces/DielectricEMInterface.h>
#include <MediumBoundaries/LensletArray.h>
#include <Surfaces/Array.h>
#include <Surfaces/Conic.h>

using namespace RZ;

// In the lenslet array we basically have an array of partially overlapped
// convex surfaces.

LensletArrayBoundary::LensletArrayBoundary()
{
  setSurfaceShape(
    new SurfaceArray(
      new ConicSurface(1e-2, m_rCurv, m_K)));

  setEMInterface(new DielectricEMInterface);
  recalculateDimensions();
}

void
LensletArrayBoundary::recalculateDimensions()
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
LensletArrayBoundary::setCurvatureRadius(Real rCurv)
{
  m_rCurv = rCurv;
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayBoundary::setRefractiveIndex(Real in, Real out)
{
  emInterface<DielectricEMInterface>()->setRefractiveIndex(in, out);
}
      
std::string
LensletArrayBoundary::name() const
{
  return "LensletArrayBoundary";
}

void
LensletArrayBoundary::setWidth(Real width)
{
  surfaceShape<SurfaceArray>()->setWidth(width);
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayBoundary::setHeight(Real height)
{
  surfaceShape<SurfaceArray>()->setHeight(height);
  m_dirty  = true;
  recalculateDimensions();
}

void
LensletArrayBoundary::setCols(unsigned cols)
{
  surfaceShape<SurfaceArray>()->setCols(cols);
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayBoundary::setConicConstant(Real K)
{
  m_K = K;
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayBoundary::setConvex(bool convex)
{
  m_convex = convex;
  m_dirty = true;
  recalculateDimensions();
}

void
LensletArrayBoundary::setRows(unsigned rows)
{
  surfaceShape<SurfaceArray>()->setRows(rows);
  m_dirty = true;
  recalculateDimensions();
}
