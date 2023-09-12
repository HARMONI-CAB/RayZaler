#include <Logger.h>

using namespace RZ;

Logger *Logger::m_logger = nullptr;

void
Logger::setDefaultLogger(Logger *logger)
{
  m_logger = logger;
}

void
Logger::log(
        LogLevel level,
        std::string const &file,
        int line,
        const char *fmt,
        ...)
{
  if (m_logger != nullptr) {
    std::va_list ap;
    va_start(ap, fmt);
    m_logger->logFunction(level, file, line, string_vprintf(fmt, ap));
    va_end(ap);
  }
}
