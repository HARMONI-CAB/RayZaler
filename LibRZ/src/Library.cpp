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

#include <Singleton.h>
#include <MediumBoundaries.h>
#include <Elements/All.h>
#include <RayTracingHeuristics/All.h>
#include <Logger.h>

#include <FT2Facade.h>

#ifdef PYTHON_SCRIPT_SUPPORT
#  include  <ScriptLoader.h>
#endif

using namespace RZ;

static bool g_initialized = false;

void
RZ::RZInit()
{
  if (g_initialized)
    return;
  g_initialized = true;

  Singleton *singleton = Singleton::instance();

  // Element factories
  singleton->registerElementFactory(new ApertureStopFactory);
  singleton->registerElementFactory(new BenchElementFactory);
  singleton->registerElementFactory(new BlockElementFactory);
  singleton->registerElementFactory(new CircularWindowFactory);
  singleton->registerElementFactory(new ConicLensFactory);
  singleton->registerElementFactory(new ConicMirrorFactory);
  singleton->registerElementFactory(new DetectorFactory);
  singleton->registerElementFactory(new IdealLensFactory);
  singleton->registerElementFactory(new FlatMirrorFactory);
  singleton->registerElementFactory(new LensletArrayFactory);
  singleton->registerElementFactory(new ObstructionFactory);
  singleton->registerElementFactory(new ParabolicLensFactory);
  singleton->registerElementFactory(new ParabolicMirrorFactory);
  singleton->registerElementFactory(new PhaseScreenFactory);
  singleton->registerElementFactory(new RayBeamElementFactory);
  singleton->registerElementFactory(new RectangularStopFactory);
  singleton->registerElementFactory(new RodElementFactory);
  singleton->registerElementFactory(new SphericalLensFactory);
  singleton->registerElementFactory(new SphericalMirrorFactory);
  singleton->registerElementFactory(new StlMeshFactory);
  singleton->registerElementFactory(new TripodFactory);
  singleton->registerElementFactory(new TubeElementFactory);

  // Ray tracing heuristics for non-sequential mode
  singleton->registerRayTracingHeuristicFactory(new DummyHeuristicFactory);

  // Static medium boundaries
  registerMediumBoundaries();

#ifdef PYTHON_SCRIPT_SUPPORT
  ScriptLoader *loader = ScriptLoader::instance();
  if (loader == nullptr)
    RZError("Failed to initialize Python VM: model scripts will not work!\n");
#else
  RZInfo("Python support disabled at compile time.\n");
#endif // PYTHON_SCRIPT_SUPPORT

  // Initialize FreeType
    
  singleton->logInitMessage();
}
