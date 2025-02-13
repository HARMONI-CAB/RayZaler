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

#ifndef _MEDIUMBOUNDARIES_H
#define _MEDIUMBOUNDARIES_H

#include "MediumBoundaries/ApertureStop.h"
#include "MediumBoundaries/ConicLens.h"
#include "MediumBoundaries/ConicMirror.h"
#include "MediumBoundaries/FlatMirror.h"
#include "MediumBoundaries/InfiniteMirror.h"
#include "MediumBoundaries/LensletArray.h"
#include "MediumBoundaries/Obstruction.h"
#include "MediumBoundaries/PassThrough.h"
#include "MediumBoundaries/PhaseScreen.h"
#include "MediumBoundaries/RectangularStop.h"
#include "MediumBoundaries/SquareFlatSurface.h"

namespace RZ {
  void registerMediumBoundaries();
}

#endif // _MEDIUMBOUNDARIES_H
