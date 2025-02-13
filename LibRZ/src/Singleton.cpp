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
#include <Element.h>
#include <MediumBoundaries.h>
#include <Logger.h>
#include <FT2Facade.h>

using namespace RZ;

Singleton *Singleton::m_currInstance = nullptr;

Singleton::Singleton()
{
  m_freeType = new FT2Facade();  
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

std::list<std::string>
Singleton::elementFactories() const
{
  std::list<std::string> list;

  for (auto &p : m_elementFactories)
    list.push_back(p.first);

  return list;
}

bool
Singleton::registerMediumBoundary(MediumBoundary *proc)
{
  std::string name = proc->name();

  if (m_MediumBoundarys.find(name) != m_MediumBoundarys.end())
    return false;

  m_MediumBoundarys.emplace(name, proc);

  return true;
}

MediumBoundary *
Singleton::lookupMediumBoundary(std::string const &name) const
{
  auto it = m_MediumBoundarys.find(name);
  if (it == m_MediumBoundarys.end())
    return nullptr;

  return it->second;
}

FT2Facade *
Singleton::freetype() const
{
  return m_freeType;
}

void
Singleton::logInitMessage()
{
  RZInfo("LibRZ core loaded (proto 0.1)\n");
  RZInfo("Global library: %d element factories, %d ray processors\n",
    m_elementFactories.size(),
    m_MediumBoundarys.size());
}
