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

#ifndef _DATA_PRODUCT_H
#define _DATA_PRODUCT_H

#include <string>

namespace RZ {
  class DataProduct {
    std::string m_name = "product";
    bool        m_haveView = false;

  protected:
    virtual void build();
    virtual void clear();

  public:
    virtual ~DataProduct();
    virtual std::string productType() const = 0;
    virtual bool saveToFile(std::string const &path) const = 0;

    void setProductName(std::string const &);
    std::string productName() const;
    void prepareView();
  };
}

#endif // _DATA_PRODUCT_H
