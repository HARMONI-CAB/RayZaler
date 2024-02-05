#ifndef _RAYPROCESSORS_H
#define _RAYPROCESSORS_H

#include "RayProcessors/ApertureStop.h"
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
