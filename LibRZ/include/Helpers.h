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

#ifndef _LIBRZ_HELPERS_H
#define _LIBRZ_HELPERS_H

#include <cstring>
#include <string>
#include <list>
#include <cstdarg>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstdint>

#define _JOIN(a, b) a ## b
#define JOIN(a, b) _JOIN(a, b)

#define _STRINGFY(a) #a
#define STRINGFY(a) _STRINGFY(a)

std::string string_vprintf(const char* fmt, va_list ap);
std::string string_printf(const char* fmt, ...);

std::vector<std::string> operator / (std::string const &, std::string const &);
std::vector<std::string> operator / (std::string const &, char sep);

static inline bool
iequals(const std::string &a, const std::string &b)
{
    return a.size() == b.size() &&
          std::equal(
            a.begin(),
            a.end(),
            b.begin(),
            [](char c_a, char c_b) {
              return std::tolower(static_cast<unsigned char>(c_a)) ==
                std::tolower(static_cast<unsigned char>(c_b));
            });
}

#if defined(__GNUC__)
#  pragma GCC push_options
#  pragma GCC optimize ("O1")
#endif 

namespace RZ {
  template<typename T>
  struct is_real {
    static constexpr bool value = false;
  };

  template<>
  struct is_real<float> {
    static constexpr bool value = true;
  };

  template<>
  struct is_real<double> {
    static constexpr bool value = true;
  };
};

template<typename T> T
sumPrecise (const T *data, size_t N)
{
  T sum = 0;

  if constexpr (RZ::is_real<T>::value) {
    T c = 0;
    T y, t;

    while (N-- > 0) {
      y = data[N] - c;
      t = sum + y;
      c = (t - sum) - y;
      sum = t;
    }
  } else {
    while (N-- > 0)
      sum += data[N];
  }

  return sum;
}

template<typename T> T
sumPrecise (std::list<T> const &data)
{
  T sum = 0;
  auto it = data.begin();

  if constexpr (RZ::is_real<T>::value) {
    T c = 0;
    T y, t;

    while (it != data.end()) {
      y = *it++ - c;
      t = sum + y;
      c = (t - sum) - y;
      sum = t;
    }
  } else {
    while (it != data.end())
      sum += *it++;
  }

  return sum;
}


#if defined(__GNUC__)
#  pragma GCC pop_options
#endif 

#endif // _LIBRZ_HELPERS_H
