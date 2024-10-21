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

#include <Helpers.h>
#include <Vector.h>

template<> RZ::Real sumPrecise(const RZ::Real *, size_t);

std::string
string_vprintf(const char* fmt, va_list ap)
{
    va_list copy;
    va_copy(copy, ap);
    
    int length = std::vsnprintf(nullptr, 0, fmt, copy);

    if (length < 0)
      throw std::runtime_error("vsnprintf() length estimation error");

    std::string s(length + 1, '\0');

    length = std::vsnprintf(s.data(), length + 1, fmt, ap);

    if (length < 0)
      throw std::runtime_error("vsnprintf() underestimation");

    va_end(copy);

    s.pop_back();
    return s;
}

std::string
string_printf(const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    auto result = string_vprintf(fmt, ap);
    va_end(ap);

    return result;
}
