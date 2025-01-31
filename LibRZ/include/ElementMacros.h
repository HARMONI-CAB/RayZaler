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


#ifndef _ELEMENT_MACROS_H
#define _ELEMENT_MACROS_H

#include "Helpers.h"

// Recursive construction of elements should work like this. Assume two constructors, 
// () and (name, desc)
//
// 1. Default constructor calls (name, desc) constructor. The latter is autogen
// 2. (name, desc) calls the parent through ()
// 3. (name, desc) sets the current metadata
// 4. () registers stuff

// In property definition, we determine whether the current metadata is up 
// to date, and push it if necessary

#define RZ_ELEMENT_FACTORY(element) JOIN(element, Factory)
#define RZ_FACTORY_METHOD(element, method) RZ_ELEMENT_FACTORY(element)::method
#define RZ_FACTORY_CTOR(element) RZ_ELEMENT_FACTORY(element)::RZ_ELEMENT_FACTORY(element)

///////////////////////// Abstract elements ////////////////////////////////////
#define RZ_DECLARE_ABSTRACT_ELEMENT_FROM(elType, fromType)                     \
  class RZ_ELEMENT_FACTORY(elType) : public RZ_ELEMENT_FACTORY(fromType) {     \
    public:                                                                    \
      RZ_ELEMENT_FACTORY(elType)();                                            \
      RZ_ELEMENT_FACTORY(elType)(std::string const &, std::string const &);    \
  }

#define RZ_DESCRIBE_ABSTRACT_ELEMENT_FROM(elType, fromType, desc)              \
RZ_FACTORY_CTOR(elType)(std::string const &name, std::string const &descStr) : \
  RZ_ELEMENT_FACTORY(fromType)()                                               \
{                                                                              \
  enterDecls(name, descStr);                                                   \
}                                                                              \
RZ_FACTORY_CTOR(elType)() : RZ_FACTORY_CTOR(elType)(STRINGFY(elType), desc)    \

/////////////////////////// Concrete elements //////////////////////////////////
#define RZ_DECLARE_ELEMENT_FROM(elType, fromType)                              \
  class RZ_ELEMENT_FACTORY(elType) : public RZ_ELEMENT_FACTORY(fromType) {     \
    public:                                                                    \
      RZ_ELEMENT_FACTORY(elType)();                                            \
      RZ_ELEMENT_FACTORY(elType)(std::string const &, std::string const &);    \
      virtual Element *make(                                                   \
        std::string const &name,                                               \
        ReferenceFrame *pFrame,                                                \
        Element *parent = nullptr) override;                                   \
  }

#define RZ_DESCRIBE_ELEMENT_FROM(elType, fromType, desc)                       \
Element *                                                                      \
RZ_FACTORY_METHOD(elType, make)(                                               \
  std::string const &name,                                                     \
  ReferenceFrame *pFrame,                                                      \
  Element *parent)                                                             \
{                                                                              \
  return new elType(this, name, pFrame, parent);                               \
}                                                                              \
RZ_DESCRIBE_ABSTRACT_ELEMENT_FROM(elType, fromType, desc)

#define RZ_DECLARE_OPTICAL_ELEMENT(elType)                                     \
  RZ_DECLARE_ELEMENT_FROM(elType, OpticalElement)
#define RZ_DESCRIBE_OPTICAL_ELEMENT(elType, desc)                              \
  RZ_DESCRIBE_ELEMENT_FROM(elType, OpticalElement, desc)

#define RZ_DECLARE_ELEMENT(elType)                                             \
  RZ_DECLARE_ELEMENT_FROM(elType, Element)
#define RZ_DESCRIBE_ELEMENT(elType, desc)                                      \
  RZ_DESCRIBE_ELEMENT_FROM(elType, Element, desc)

#define RZ_DECLARE_ABSTRACT_ELEMENT(elType)                                    \
  RZ_DECLARE_ABSTRACT_ELEMENT_FROM(elType, Element)
#define RZ_DESCRIBE_ABSTRACT_ELEMENT(elType, desc)                             \
  RZ_DESCRIBE_ABSTRACT_ELEMENT_FROM(elType, Element, desc)


#endif // _ELEMENT_MACROS_H
