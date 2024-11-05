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

#ifndef _DATA_PRODUCTS_SCATTER_H
#define _DATA_PRODUCTS_SCATTER_H

#include <DataProduct.h>
#include <list>

namespace RZ {
  class  ScatterTree;
  class  ScatterTreeRenderer;
  struct OpticalSurface;

  class ScatterSet {
    std::string  m_label = "No name";
    ScatterTree *m_tree = nullptr; // Owned
    uint32_t     m_id = 0;
    size_t       m_size;

  public:
    ScatterSet(uint32_t id, OpticalSurface const *, std::string const &label = "");
    ~ScatterSet();

    void rebuild();
    void render(ScatterTreeRenderer *) const;
    size_t size() const;
  };

  class ScatterDataProduct : public DataProduct {
      std::list<ScatterSet *> m_setList;
      unsigned int            m_idCount = 0;
      size_t                  m_points = 0;

    public:
      ScatterDataProduct(std::string const &);
      virtual ~ScatterDataProduct() override;

      virtual void clear() override;
      virtual void build() override;
      virtual std::string productType() const override;
      virtual bool saveToFile(std::string const &path) const override;

      size_t points() const;
      void render(ScatterTreeRenderer *) const;
      size_t size() const;
      void addSurface(OpticalSurface const *, std::string const &label = "");
      void addSurface(uint32_t id, OpticalSurface const *, std::string const &label = "");
  };
};

#endif // _DATA_PRODUCTS_SCATTER_H
