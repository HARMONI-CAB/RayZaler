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

#include <DataProduct.h>

using namespace RZ;

DataProduct::~DataProduct()
{
  
}

void
DataProduct::setProductName(std::string const &name)
{
  m_name = name;
}

std::string
DataProduct::productName() const
{
  return m_name;
}

void
DataProduct::prepareView()
{
  if (!m_haveView) {
    build();
    m_haveView = true;
  }
}

void
DataProduct::build()
{

}

void
DataProduct::clear()
{

}
