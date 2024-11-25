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

#ifndef _RAYPROCESSORS_H
#define _RAYPROCESSORS_H

#include "RayProcessors/ApertureStop.h"
#include "RayProcessors/ConicMirror.h"
#include "RayProcessors/FlatMirror.h"
#include "RayProcessors/InfiniteMirror.h"
#include "RayProcessors/LensletArray.h"
#include "RayProcessors/Obstruction.h"
#include "RayProcessors/ParabolicMirror.h"
#include "RayProcessors/PassThrough.h"
#include "RayProcessors/PhaseScreen.h"
#include "RayProcessors/RectangularStop.h"
#include "RayProcessors/SphericalLens.h"
#include "RayProcessors/SphericalMirror.h"
#include "RayProcessors/SquareFlatSurface.h"

namespace RZ {
  void registerRayProcessors();
}

#endif // _RAYPROCESSORS_H
