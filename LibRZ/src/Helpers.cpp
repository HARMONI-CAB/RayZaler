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
#include <vector>

template<> RZ::Real sumPrecise(const RZ::Real *, size_t);
template<> RZ::Vec3 sumPrecise(const RZ::Vec3 *, size_t);

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

std::vector<std::string>
string_split(const std::string &str, const std::string &sep)
{
  size_t posStart = 0, posEnd, delimLen = sep.length();
  std::string token;
  std::vector<std::string> res;

  while ((posEnd = str.find(sep, posStart)) != std::string::npos) {
    token = str.substr(posStart, posEnd - posStart);
    posStart = posEnd + delimLen;
    res.push_back(token);
  }

  res.push_back(str.substr(posStart));
  return res;
}

std::vector<std::string>
string_split(const std::string &str, char c)
{
  size_t posStart = 0, posEnd;
  std::string token;
  std::vector<std::string> res;

  while ((posEnd = str.find(c, posStart)) != std::string::npos) {
    token = str.substr(posStart, posEnd - posStart);
    posStart = posEnd + 1;
    res.push_back(token);
  }

  res.push_back(str.substr(posStart));
  return res;
}

std::vector<std::string>
operator / (std::string const &str, std::string const &sep)
{
  return string_split(str, sep);
}

std::vector<std::string>
operator / (std::string const &str, char c)
{
  return string_split(str, c);
}
