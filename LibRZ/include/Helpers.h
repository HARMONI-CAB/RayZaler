#ifndef _LIBRZ_HELPERS_H
#define _LIBRZ_HELPERS_H

#include <cstring>
#include <string>
#include <cstdarg>
#include <stdexcept>
#include <type_traits>

std::string string_vprintf(const char* fmt, va_list ap);
std::string string_printf(const char* fmt, ...);

#if defined(__GNUC__)
#  pragma GCC push_options
#  pragma GCC optimize ("O1")
#endif 

template<typename T> T
sumPrecise (const T *data, size_t N)
{
  T sum = 0;

  if constexpr (std::is_floating_point<T>()) {
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

#if defined(__GNUC__)
#  pragma GCC pop_options
#endif 

#endif // _LIBRZ_HELPERS_H
