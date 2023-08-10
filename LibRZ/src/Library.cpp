#include <Singleton.h>
#include <RayProcessors.h>

#include <BenchElement.h>
#include <CircularMirror.h>
#include <ConcaveMirror.h>
#include <BlockElement.h>
#include <RayBeamElement.h>
#include <Detector.h>

using namespace RZ;

void
RZ::RZInit()
{
  Singleton *singleton = Singleton::instance();

  Singleton::instance()->registerElementFactory(new BenchElementFactory);
  Singleton::instance()->registerElementFactory(new CircularMirrorFactory);
  Singleton::instance()->registerElementFactory(new ConcaveMirrorFactory);
  Singleton::instance()->registerElementFactory(new DetectorFactory);
  Singleton::instance()->registerElementFactory(new RayBeamElementFactory);
  Singleton::instance()->registerElementFactory(new BlockElementFactory);
  
  registerRayProcessors();
}
