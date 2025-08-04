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

#include <MediumBoundary.h>
#include <SurfaceShape.h>
#include <EMInterface.h>
#include <RayTracingEngine.h>
#include <Logger.h>

using namespace RZ;

static const EMMedium g_vacuum;

/////////////////////////////// MediumBoundary ///////////////////////////
MediumBoundary::~MediumBoundary()
{
  if (m_surfaceShape != nullptr)
    delete m_surfaceShape;

  if (m_emInterface != nullptr)
    delete m_emInterface;
}

void
MediumBoundary::setParentFrame(const ReferenceFrame *frame)
{
  m_frame = frame;

  if (m_emInterface != nullptr)
    m_emInterface->setParentFrame(frame);
}

void
MediumBoundary::cast(RayBeamSlice const &slice) const
{
  Vec3 destination;
  auto &beam = *slice.beam;
  uint64_t end = slice.end;
  Real dt;
  
  auto shape     = surfaceShape();

  if (shape != nullptr) {
    bool first = false;
    for (uint64_t i = slice.start; i < end; ++i) {
      if (beam.hasRay(i)) {
        Vec3 origin = Vec3(beam.origins + 3 * i);
        Vec3 dir    = Vec3(beam.directions + 3 * i);
        Vec3 normal;

        // Do intercept. Note we do not do pruning here.
        if (surfaceShape()->intercept(destination, normal, dt, origin, dir)) {
          if (!clipped(destination.x, destination.y)) {
            beam.lengths[i]   = dt;
            beam.opl[i]      += beam.neff[i] * dt;

            if (beam.fields) {
              auto K = 2 * M_PI * beam.neff[i] / beam.wavelengths[i];
              auto dPhi = std::exp(Complex(0, K * dt));

              beam.Dx[i] *= dPhi;
              beam.Dy[i] *= dPhi;
            }

            destination.copyToArray(beam.destinations + 3 * i);
            normal.copyToArray(beam.normals + 3 * i);
            beam.intercept(i);
          }
        }
      }
    }
  } else {
    // Surface is infinite and flat
    for (uint64_t i = slice.start; i < end; ++i) {
      if (beam.hasRay(i)) {
        Vec3 origin = Vec3(beam.origins + 3 * i);
        Vec3 dir    = Vec3(beam.directions + 3 * i);

        // Intercept only if the ray is not parallel to the surface
        if (!isZero(dir.z)) {
          dt                     = -origin.z / dir.z;
          destination            = origin + dt * dir;
          
          if (!clipped(destination.x, destination.y)) {
            beam.lengths[i]   = dt;
            beam.opl[i]      += beam.neff[i] * dt;
            
            if (beam.fields) {
              auto K = 2 * M_PI * beam.neff[i] / beam.wavelengths[i];
              auto dPhi = std::exp(Complex(0, K * dt));

              beam.Dx[i] *= dPhi;
              beam.Dy[i] *= dPhi;
            }
            
            destination.copyToArray(beam.destinations + 3 * i);
            Vec3::eZ().copyToArray(beam.normals + 3 * i);
            beam.intercept(i);
          }
        }
      }
    }
  }
}

void
MediumBoundary::transmit(RayBeamSlice const &slice, RayBeam *splinterRays) const
{
  // Transmit rays through this interface with the intercepted beams
  if (emInterface() != nullptr)
    emInterface()->transmit(slice, splinterRays);
}
