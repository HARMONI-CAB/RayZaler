#include <Singleton.h>
#include <RayProcessors.h>

#include <BenchElement.h>
#include <FlatMirror.h>
#include <SphericalMirror.h>
#include <BlockElement.h>
#include <RayBeamElement.h>
#include <ConvexLens.h>
#include <Detector.h>

using namespace RZ;

void
RZ::RZInit()
{
  Singleton *singleton = Singleton::instance();

  Singleton::instance()->registerElementFactory(new BenchElementFactory);
  Singleton::instance()->registerElementFactory(new FlatMirrorFactory);
  Singleton::instance()->registerElementFactory(new SphericalMirrorFactory);
  Singleton::instance()->registerElementFactory(new DetectorFactory);
  Singleton::instance()->registerElementFactory(new RayBeamElementFactory);
  Singleton::instance()->registerElementFactory(new BlockElementFactory);
  Singleton::instance()->registerElementFactory(new ConvexLensFactory);
  
  registerRayProcessors();
}
