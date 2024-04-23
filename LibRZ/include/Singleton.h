#ifndef _SINGLETON_H
#define _SINGLETON_H

#include <string>
#include <list>
#include <map>

namespace RZ
{
  class ElementFactory;
  class RayTransferProcessor;
  class FT2Facade;

  class Singleton {
      std::map<std::string, ElementFactory *> m_elementFactories;
      std::map<std::string, RayTransferProcessor *> m_rayTransferProcessors;
      FT2Facade *m_freeType = nullptr;

      static Singleton *m_currInstance;

      Singleton();

    public:
      static Singleton *instance();
      bool registerElementFactory(ElementFactory *);
      bool registerRayTransferProcessor(RayTransferProcessor *);

      ElementFactory *lookupElementFactory(std::string const &) const;
      RayTransferProcessor *lookupRayTransferProcessor(std::string const &) const;
      FT2Facade *freetype() const;

      void logInitMessage();
  };

  void RZInit();
}

#endif // _SINGLETON_H
