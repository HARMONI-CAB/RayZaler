#include <RayProcessors.h>
#include <Singleton.h>

void
RZ::registerRayProcessors()
{
  Singleton::instance()->registerRayTransferProcessor(new PassThroughProcessor);
  Singleton::instance()->registerRayTransferProcessor(new InfiniteMirrorProcessor);
}
