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

#ifndef _PARABOLIC_MIRROR_H
#define _PARABOLIC_MIRROR_H

#include <OpticalElement.h>
#include <ConicMirror.h>
#include <RayProcessors.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;

  class ParabolicMirror : public ConicMirror {
    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      ParabolicMirror(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      ~ParabolicMirror();
  };

  class ParabolicMirrorFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _PARABOLIC_MIRROR_H
