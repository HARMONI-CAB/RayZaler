#ifndef _LIBRZ_HELPERS_H
#define _LIBRZ_HELPERS_H

#include <cstring>
#include <string>
#include <cstdarg>
#include <stdexcept>

std::string string_vprintf(const char* fmt, va_list ap);
std::string string_printf(const char* fmt, ...);

#endif // _LIBRZ_HELPERS_H
