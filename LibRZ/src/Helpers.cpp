#include <Helpers.h>

std::string string_vprintf(const char* fmt, va_list ap)
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
