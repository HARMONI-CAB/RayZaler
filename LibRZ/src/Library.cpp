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
#include <SphericalMirror.h>

using namespace RZ;

void
RZ::RZInit()
{
  Singleton *singleton = Singleton::instance();

  Singleton::instance()->registerElementFactory(new ApertureStopFactory);
  Singleton::instance()->registerElementFactory(new BenchElementFactory);
  Singleton::instance()->registerElementFactory(new BlockElementFactory);
  Singleton::instance()->registerElementFactory(new ConvexLensFactory);
  Singleton::instance()->registerElementFactory(new DetectorFactory);
  Singleton::instance()->registerElementFactory(new FlatMirrorFactory);
  Singleton::instance()->registerElementFactory(new ObstructionFactory);
  Singleton::instance()->registerElementFactory(new RayBeamElementFactory);
  Singleton::instance()->registerElementFactory(new SphericalMirrorFactory);
  
  registerRayProcessors();
}
