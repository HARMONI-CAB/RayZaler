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

using namespace RZ;

/////////////////////////////// MediumBoundary ///////////////////////////
MediumBoundary::~MediumBoundary()
{
  if (m_surfaceShape != nullptr)
    delete m_surfaceShape;

  if (m_emInterface != nullptr)
    delete m_emInterface;
}

void
MediumBoundary::cast(RayBeam &beam) const
{
  Vec3 destination;
  uint64_t count = beam.count;
  Real K, dt, opd;
  bool comp      = m_complementary;
  auto shape     = surfaceShape();

  if (shape != nullptr) {
    for (uint64_t i = 0; i < count; ++i) {
      if (beam.hasRay(i)) {
        Vec3 origin = Vec3(beam.origins + 3 * i);
        Vec3 dir    = Vec3(beam.directions + 3 * i);
        Vec3 normal;
        Real dt, opd;

        // Do intercept. Note we do not do pruning here.
        if (surfaceShape()->intercept(destination, normal, dt, origin, dir) != comp) {
          K                      = 2 * M_PI / beam.wavelengths[i];
          opd                    = beam.refNdx[i] * dt;
          beam.lengths[i]        = dt;
          beam.cumOptLengths[i] += opd;
          beam.amplitude[i]     *= std::exp(Complex(0, K * opd));

          destination.copyToArray(beam.destinations + 3 * i);
          normal.copyToArray(beam.normals     + 3 * i);
          beam.intercept(i);
        }
      }
    }
  } else {
    // Surface is infinite and flat
    if (!comp) {
      for (uint64_t i = 0; i < count; ++i) {
        if (beam.hasRay(i)) {
          Vec3 origin = Vec3(beam.origins + 3 * i);
          Vec3 dir    = Vec3(beam.directions + 3 * i);
          
          // Intercept only if the ray is not parallel to the surface
          if (!isZero(dir.z)) {
            dt                     = -origin.z / dir.z;
            destination            = origin + dt * dir;
            
            K                      = 2 * M_PI / beam.wavelengths[i];
            opd                    = beam.refNdx[i] * dt;
            beam.lengths[i]        = dt;
            beam.cumOptLengths[i] += opd;
            beam.amplitude[i]     *= std::exp(Complex(0, K * opd));

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
MediumBoundary::transmit(RayBeamSlice const &slice) const
{
  // Transmit rays through this interface with the intercepted beams
  if (emInterface() != nullptr)
    emInterface()->transmit(slice);
}
