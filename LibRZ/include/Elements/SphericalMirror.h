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

#ifndef _SPHERICAL_MIRROR_H
#define _SPHERICAL_MIRROR_H

#include "ConicMirror.h"

namespace RZ {
  class TranslatedFrame;

  class SphericalMirror : public ConicMirror {
    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      SphericalMirror(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~SphericalMirror() override;
  };

  RZ_DECLARE_ELEMENT_FROM(SphericalMirror, ConicMirror);
}

#endif // _SPHERICAL_MIRROR_H
