#include <Singleton.h>
#include <Element.h>
#include <RayProcessors.h>
#include <Logger.h>

using namespace RZ;

Singleton *Singleton::m_currInstance = nullptr;

Singleton::Singleton()
{
}

Singleton *
Singleton::instance()
{
  if (m_currInstance == nullptr) {
    m_currInstance = new Singleton();
    RZInit();
  }

  return m_currInstance;
}

bool
Singleton::registerElementFactory(ElementFactory *factory)
{
  std::string name = factory->name();

  if (m_elementFactories.find(name) != m_elementFactories.end())
    return false;

  m_elementFactories.emplace(name, factory);

  return true;
}

ElementFactory *
Singleton::lookupElementFactory(std::string const &name) const
{
  auto it = m_elementFactories.find(name);
  if (it == m_elementFactories.end())
    return nullptr;

  return it->second;
}

bool
Singleton::registerRayTransferProcessor(RayTransferProcessor *proc)
{
  std::string name = proc->name();

  if (m_rayTransferProcessors.find(name) != m_rayTransferProcessors.end())
    return false;

  m_rayTransferProcessors.emplace(name, proc);

  return true;
}

RayTransferProcessor *
Singleton::lookupRayTransferProcessor(std::string const &name) const
{
  auto it = m_rayTransferProcessors.find(name);
  if (it == m_rayTransferProcessors.end())
    return nullptr;

  return it->second;
}

void
Singleton::logInitMessage()
{
  RZInfo("LibRZ core loaded (proto 0.1)\n");
  RZInfo("Global library: %d element factories, %d ray processors\n",
    m_elementFactories.size(),
    m_rayTransferProcessors.size());
}
