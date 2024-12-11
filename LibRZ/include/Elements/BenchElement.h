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

#ifndef _BENCHELEMENT_H
#define _BENCHELEMENT_H

#include <Element.h>
#include <GL/glu.h>
#include <GLHelpers.h>

namespace RZ {
  class TranslatedFrame;
  class BenchElement : public Element {
      TranslatedFrame *m_surfaceFrame = nullptr;
      Real m_cachedHeight             = 0;
      GLCappedCylinder m_cylinder;
      
    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
      BenchElement(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      
      virtual ~BenchElement() override;
      virtual void nativeMaterialOpenGL(std::string const &role) override;
      virtual void renderOpenGL() override;
  };

  class BenchElementFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _BENCHELEMENT_H
