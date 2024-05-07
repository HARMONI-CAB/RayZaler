#include <Singleton.h>
#include <RayProcessors.h>
#include <ApertureStop.h>
#include <BenchElement.h>
#include <BlockElement.h>
#include <CircularWindow.h>
#include <ConvexLens.h>
#include <Detector.h>
#include <FlatMirror.h>
#include <LensletArray.h>
#include <Obstruction.h>
#include <ParabolicMirror.h>
#include <PhaseScreen.h>
#include <RayBeamElement.h>
#include <RectangularStop.h>
#include <RodElement.h>
#include <SphericalMirror.h>
#include <StlMesh.h>
#include <Tripod.h>
#include <TubeElement.h>
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

  Singleton::instance()->registerElementFactory(new ApertureStopFactory);
  Singleton::instance()->registerElementFactory(new BenchElementFactory);
  Singleton::instance()->registerElementFactory(new BlockElementFactory);
  Singleton::instance()->registerElementFactory(new CircularWindowFactory);
  Singleton::instance()->registerElementFactory(new ConvexLensFactory);
  Singleton::instance()->registerElementFactory(new DetectorFactory);
  Singleton::instance()->registerElementFactory(new FlatMirrorFactory);
  Singleton::instance()->registerElementFactory(new LensletArrayFactory);
  Singleton::instance()->registerElementFactory(new ObstructionFactory);
  Singleton::instance()->registerElementFactory(new ParabolicMirrorFactory);
  Singleton::instance()->registerElementFactory(new PhaseScreenFactory);
  Singleton::instance()->registerElementFactory(new RayBeamElementFactory);
  Singleton::instance()->registerElementFactory(new RectangularStopFactory);
  Singleton::instance()->registerElementFactory(new RodElementFactory);
  Singleton::instance()->registerElementFactory(new SphericalMirrorFactory);
  Singleton::instance()->registerElementFactory(new StlMeshFactory);
  Singleton::instance()->registerElementFactory(new TripodFactory);
  Singleton::instance()->registerElementFactory(new TubeElementFactory);

  registerRayProcessors();

#ifdef PYTHON_SCRIPT_SUPPORT
  ScriptLoader *loader = ScriptLoader::instance();
  if (loader == nullptr)
    RZError("Failed to initialize Python VM: model scripts will not work!\n");
#else
  RZInfo("Python support disabled at compile time.\n");
#endif // PYTHON_SCRIPT_SUPPORT

  // Initialize FreeType
    
  Singleton::instance()->logInitMessage();
}
