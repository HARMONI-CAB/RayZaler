#ifndef _SINGLETON_H
#define _SINGLETON_H

#include <string>
#include <list>
#include <map>

namespace RZ
{
  class ElementFactory;
  class RayTransferProcessor;

  class Singleton {
      std::map<std::string, ElementFactory *> m_elementFactories;
      std::map<std::string, RayTransferProcessor *> m_rayTransferProcessors;
      static Singleton *m_currInstance;

      Singleton();

    public:
      static Singleton *instance();
      bool registerElementFactory(ElementFactory *);
      bool registerRayTransferProcessor(RayTransferProcessor *);

      ElementFactory *lookupElementFactory(std::string const &) const;
      RayTransferProcessor *lookupRayTransferProcessor(std::string const &) const;
  };

  void RZInit();
}

#endif // _SINGLETON_H
