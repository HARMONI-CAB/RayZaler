#include <Singleton.h>
#include <RayProcessors.h>

#include <ApertureStop.h>
#include <BenchElement.h>
#include <BlockElement.h>
#include <ConvexLens.h>
#include <Detector.h>
#include <FlatMirror.h>
#include <Obstruction.h>
#include <RayBeamElement.h>
#include <RodElement.h>
#include <SphericalMirror.h>
#include <Tripod.h>
#include <TubeElement.h>

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
  Singleton::instance()->registerElementFactory(new ConvexLensFactory);
  Singleton::instance()->registerElementFactory(new DetectorFactory);
  Singleton::instance()->registerElementFactory(new FlatMirrorFactory);
  Singleton::instance()->registerElementFactory(new ObstructionFactory);
  Singleton::instance()->registerElementFactory(new RayBeamElementFactory);
  Singleton::instance()->registerElementFactory(new RodElementFactory);
  Singleton::instance()->registerElementFactory(new SphericalMirrorFactory);
  Singleton::instance()->registerElementFactory(new TripodFactory);
  Singleton::instance()->registerElementFactory(new TubeElementFactory);

  registerRayProcessors();

  Singleton::instance()->logInitMessage();
}
