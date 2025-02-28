//
//  Copyright (c) 2025 Gonzalo José Carracedo Carballal
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

#include <Common.h>

using namespace RZ;

void
BeamTestStatistics::computeFromSurface(
  OpticalSurface const *fp,
  Vec3 const &chiefRay)
{
  Real R2         = 0;
  auto locations  = fp->locations();
  auto directions = fp->directions();
  size_t N        = locations.size() / 3;

  const Vec3 *locVec = reinterpret_cast<const Vec3 *>(locations.data());
  const Vec3 *dirVec = reinterpret_cast<const Vec3 *>(directions.data());

  Vec3 center = sumPrecise<Vec3>(locVec, N);

  x0 = center.x / N;
  y0 = center.y / N;

  Real corr = 0, c = 0, t;
  Real currFnum;

  
  for (size_t i = 3; i < locations.size(); i += 3) {
    Real x = locations[i] - x0;
    Real y = locations[i + 1] - y0;

    R2     = x * x + y * y;
    maxRad = fmax(R2, maxRad);
    fNum   = fabsmin(.5 / tan(acos(dirVec[i / 3] * chiefRay)), fNum);

    corr   = R2 - c;
    t      = rmsRad + corr;
    c      = (t - rmsRad) - corr;
    rmsRad = t;
  }

  rmsRad = sqrt(rmsRad / static_cast<Real>(N));
  maxRad = sqrt(maxRad);

  vignetted = intercepted = pruned = 0;

  for (auto &p : fp->statistics) {
    intercepted += p.second.intercepted;
    vignetted   += p.second.vignetted;
    pruned      += p.second.pruned;
  }
}

void
BeamTestStatistics::computeFromRayList(
  std::list<Ray> const &rays,
  Vec3 const &chiefRay)
{
  Real  R2     = 0;
  size_t N     =  rays.size();

  Vec3 vCorr, vC, vT;
  Vec3 center;

  for (auto const &ray : rays) {
    vCorr  = ray.origin - vC;
    vT     = center + vCorr;
    vC     = (vT - center) - vCorr;
    center = vT;
  }

  x0 = center.x / N;
  y0 = center.y / N;

  Real corr = 0, c = 0, t;

  intercepted = vignetted = pruned = 0;
  
  for (auto const &ray : rays) {
    Real x = ray.origin.x - x0;
    Real y = ray.origin.y - y0;

    R2     = x * x + y * y;
    maxRad = fmax(R2, maxRad);
    fNum   = fabsmin(.5 / tan(acos(ray.direction * chiefRay)), fNum);

    corr   = R2 - c;
    t      = rmsRad + corr;
    c      = (t - rmsRad) - corr;
    rmsRad = t;

    if (ray.intercepted)
      ++intercepted;
    else
      ++vignetted;
  }

  rmsRad = sqrt(rmsRad / static_cast<Real>(N));
  maxRad = sqrt(maxRad);
}
