#ifndef _LIBRZ_HELPERS_H
#define _LIBRZ_HELPERS_H

#include <cstring>
#include <string>
#include <list>
#include <cstdarg>
#include <stdexcept>
#include <type_traits>
#include <vector>

std::string string_vprintf(const char* fmt, va_list ap);
std::string string_printf(const char* fmt, ...);

std::vector<std::string> operator / (std::string const &, std::string const &);
std::vector<std::string> operator / (std::string const &, char sep);

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
