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

#ifndef _SINGLETON_H
#define _SINGLETON_H

#include <string>
#include <list>
#include <map>

namespace RZ
{
  class ElementFactory;
  class MediumBoundary;
  class FT2Facade;

  class Singleton {
      std::map<std::string, ElementFactory *>       m_elementFactories;
      std::map<std::string, MediumBoundary *> m_MediumBoundarys;
      FT2Facade *m_freeType = nullptr;

      static Singleton *m_currInstance;

      Singleton();

    public:
      static Singleton *instance();
      bool registerElementFactory(ElementFactory *);
      bool registerMediumBoundary(MediumBoundary *);

      ElementFactory *lookupElementFactory(std::string const &) const;
      std::list<std::string> elementFactories() const;
      
      MediumBoundary *lookupMediumBoundary(std::string const &) const;
      FT2Facade *freetype() const;

      void logInitMessage();
  };

  void RZInit();
}

#endif // _SINGLETON_H
